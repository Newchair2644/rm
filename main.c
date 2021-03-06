#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

// check if dir
int isfile(const char *name)
{
	DIR *directory = opendir(name);
	// is dir
	if (directory != NULL) {
		closedir(directory);
		return 0;
	}
	// is file
	if(errno == ENOTDIR)
		return 1;
	// does not exist
	return -1;
}

int main(int argc, char *argv[])
{
	int mode[3] = { 0, 0, 0 };
	enum { force, interactive, recursive };

	// No arguments given
	if (argc <= 1) {
		fprintf(stderr, "rm: missing operand\n" \
				"Try rm -h for more information\n");
		return 1;
	}

	size_t optind, optinl;
	for (optind = 1; optind < argc && argv[optind][0] == '-'; optind++) {
		for (optinl = 1;  optinl < strlen(argv[optind]); optinl++) {
			switch (argv[optind][optinl]) {
				// Force mode: remove files even if write-protected
				case 'f': mode[force] = 1; break;
				// Interactive mode: prompt for confirmation
				case 'i': mode[interactive] = 1; break;
				// Recursive mode: remove files and directories
				case 'R': case 'r': mode[recursive] = 1; break;
				default:
					fprintf(stderr, "Usage: rm [-fiRr]... [FILE...]\n");
					return 1;
			}
		}
	}
	argv += optind;

	int status = 0;
	for (char **file = argv; *file; file++) {
		if (!access(*file, F_OK) && !mode[force]) {
			fprintf(stderr, "rm: cannot remove '%s' No such file or directory\n", *file);
			status = 1;
			continue;
		} else if (!isfile(*file) && !mode[recursive]) {
			fprintf(stderr, "rm: cannot remove '%s': Is a directory\n", *file);
			status = 1;
			continue;
		} else if (mode[interactive] || (!access(*file, W_OK) && !mode[force])) {
			fprintf(stderr, "rm: remove '%s'? ", *file);
			if (getchar() != 'y')
				continue;
		}

		remove(*file);
	}

	return status;
}