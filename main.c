#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

int mode[4] = { 0, 0, 0, 0 };
enum { NORMAL_MODE, FORCE_MODE, INTERACTIVE_MODE, RECURSIVE_MODE };

void confirm(char *file);
int isfile(const char *name);

int main(int argc, char *argv[])
{
	// No arguments given
	if (argc <= 1) {
		fprintf(stderr, "rm: missing operand\n" \
				"Try rm -h for more information\n");
		return EXIT_FAILURE;
	}

	size_t optind;
	for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
		for (size_t optinl = 1;  optinl < strlen(argv[optind]); optinl++) {
			switch (argv[optind][optinl]) {
				// Force mode: remove files even if write-protected
				case 'f': mode[FORCE_MODE] = 1; break;
				// Interactive mode: prompt for confirmation
				case 'i': mode[INTERACTIVE_MODE] = 1; break;
				// Recursive mode: remove files and directories
				case 'R': case 'r': mode[RECURSIVE_MODE] = 1; break;
				default:
					fprintf(stderr, "Usage: rm [-fiRr]... [FILE...]\n");
					return EXIT_FAILURE;
			}
		}
	}
	if (optind == 1)
		mode[NORMAL_MODE] = 1;

	argv += optind; // increment by the number of options, leaving us with the files
	int incorrect_file = 0; // wheather or not user has made an incorrect input
	for (char **file = argv; *file; file++) {
		if (access(*file, F_OK) != 0 && !mode[FORCE_MODE]) {
			fprintf(stderr, "rm: cannot remove '%s' No such file or directory\n", *file);
			incorrect_file = 1;
			break;
		}

		if (mode[FORCE_MODE] && (mode[RECURSIVE_MODE] || isfile(*file))) {
			remove(*file);
		} else if (mode[INTERACTIVE_MODE] || access(*file, W_OK) != 0) {
			confirm(*file);
			// switch to normal mode so we still check if it is a file (unless recursive mode is on)
			if (!mode[RECURSIVE_MODE])
				mode[NORMAL_MODE] = 1;
		}
		// Normal mode (default): remove files if they exist or if they are not dirs
		if ((mode[NORMAL_MODE] && isfile(*file)) || mode[RECURSIVE_MODE])
			remove(*file);
		else
			incorrect_file = 1;

	}
	// if any of the files were invalid, return failure signal
	return incorrect_file ? EXIT_FAILURE : EXIT_SUCCESS;
}

// confim deletion
void confirm(char *file)
{
	fprintf(stderr, "rm: remove '%s'? ", file);
	char confirm = getchar();
	if (confirm != 'y')
		exit(EXIT_FAILURE);
}

// check if dir
int isfile(const char *name)
{
	DIR *directory = opendir(name);
	// is dir
	if (directory != NULL) {
		closedir(directory);
		fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", name);
		return 0;
	}
	// is file
	if(errno == ENOTDIR)
		return 1;
	// does not exist
	return -1;
}