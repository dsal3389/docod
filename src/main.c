#include <alloca.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dll.h"
#include "lexer.h"

const char *project_path = "./src";
const char *doc_output = "./docs";

void fatal(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  exit(-1);
}

struct Lexer *get_lexer_for_file(const char *path) {
  struct Lexer *lxr = malloc(sizeof(struct Lexer));
  struct stat sb;
  int fd;

  if (stat(path, &sb) != 0) {
    fatal("couldn't get file stat for `%s`\n", path);
  }

  if ((fd = open(path, O_RDONLY)) == -1) {
    fatal("couldn't open project file at `%s`\n", path);
  }

  const char *content = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  close(fd);

  lexer_init(lxr, content);
  return lxr;
}

void document_project_files(const char **paths, const char *output) {
  errno = 0;
  if (mkdir(output, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) != 0) {
    if (errno != EEXIST) {
      fatal("couldn't create output folder name `%s`\n", output);
    }
  }

  for (const char **path = paths; *path != NULL; path++) {
    struct Lexer *lxr = get_lexer_for_file(*path);
    struct LexerToken tkn;

    lexer_token_init(&tkn);

    while (lexer_next_token(lxr, &tkn)) {
      const char *string = lexer_token_position_string(lxr, &tkn);
      printf("(%s) %s\n", lexer_token_type_to_string(tkn.type), string);
    }
  }
}

void discover_project_files(const char *basedir, char ***buffer) {
  struct DoubleLinkedList stack;
  struct DoubleLinkedList files;
  struct Node *current = NULL;
  struct stat stat_buffer;

  struct Node tmp;
  dll_init(&stack);
  dll_init(&files);
  dll_node_init(&tmp, (void *)basedir);
  dll_push(&stack, &tmp);

  // iterate over folders, if we find files we
  // push it to the `files` stack for later process
  while ((current = dll_pop(&stack)) != NULL) {
    const char *path = (char *)current->value;

    if (stat(path, &stat_buffer) == -1) {
      fatal("couldn't get file stat `%s`\n", path);
    }

    switch (stat_buffer.st_mode & S_IFMT) {
    case S_IFBLK:
    case S_IFREG:
    case S_IFCHR:
      // pushing the current node the the `files` list
      dll_push(&files, current);
      break;
    case S_IFDIR:
      DIR *dir = NULL;
      struct dirent *entry = NULL;

      if ((dir = opendir(path)) == NULL) {
        fatal("couldn't open directory at %s\n", path);
      }

      while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
          continue;
        }

        char *entry_path = calloc(sizeof(char), PATH_MAX);
        snprintf(entry_path, PATH_MAX, "%s/%s", path, entry->d_name);

        struct Node *node = alloca(sizeof(struct Node));
        dll_node_init(node, entry_path);
        dll_push(&stack, node);
      }
      closedir(dir);
      break;
    default:
      break;
    }
  }

  *buffer = malloc(sizeof(char *) * files.length + 1);

  // set last element in list as null to indicate end of the list
  (*buffer)[files.length] = NULL;
  size_t buffer_index = 0;

  while ((current = dll_pop(&files)) != NULL) {
    (*buffer)[buffer_index++] = (char *)current->value;
  }
}

int main(int argc, char **argv) {
  char **paths = NULL;
  discover_project_files(project_path, &paths);
  document_project_files((const char **)paths, doc_output);
}
