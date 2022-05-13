/*
 * ls.c
 * Implementation of the Unix ls command using the stat() system call,
 * instead of statx().
 *
 * Author: Marcelo A F Gomes (May 2022)
 *
 * This source code is distributed under the GNU General Public License v2.
 * Please be sure to read the LICENSE file to know what you may and may not
 * do with this source code.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

// Flag to indicate we're debugging, instead of creating
// a production-quality version of the software
#define DEBUGGING 0

// Codes to encode the command-line options inside
// the 'flags' global variable
#define OPT_l	0x01
#define OPT_a	0x02

// Global variable to hold the encoded command-line options
static int flags = 0;

// Macros to test the options stored in the 'flags' variable
#ifdef DEBUGGING
#if DEBUGGING
#define flag_ativo(f)	((flags & f)?"ON":"OFF")
#endif // DEBUGGING
#endif // def DEBUGGING
#define flag(f)		((flags & f)?1:0)

void print_mode(mode_t m) {
    register char ftype;

    switch( m & S_IFMT ) {
    case S_IFDIR:  ftype = 'd'; break;
    case S_IFIFO:  ftype = 'f'; break;
    case S_IFLNK:  ftype = 'l'; break;
    case S_IFSOCK: ftype = 's'; break;
    case S_IFBLK:  ftype = 'b'; break;
    case S_IFCHR:  ftype = 'c'; break;
    case S_IFREG:  ftype = '-'; break;
    default:       ftype = '?'; break;
    }
    printf("%c", ftype);

    // User bits
    if( m & S_IRUSR )
	printf("r");
    else
	printf("-");

    if( m & S_IWUSR )
	printf("w");
    else
	printf("-");

    if( m & S_ISUID ) {
	if( m & S_IXUSR )
	    printf("s");
	else
	    printf("S");
    } else {
	if( m & S_IXUSR )
	    printf("x");
	else
	    printf("-");
    }

    // Group bits
    if( m & S_IRGRP )
	printf("r");
    else
	printf("-");

    if( m & S_IWGRP )
	printf("w");
    else
	printf("-");

    if( m & S_ISGID ) {
	if( m & S_IXGRP )
	    printf("s");
	else
	    printf("S");
    } else {
	if( m & S_IXGRP )
	    printf("x");
	else
	    printf("-");
    }

    // Other bits
    if( m & S_IROTH )
	printf("r");
    else
	printf("-");

    if( m & S_IWOTH )
	printf("w");
    else
	printf("-");

    if( m & S_ISVTX ) {
	if( m & S_IXOTH )
	    printf("t");
	else
	    printf("T");
    } else {
	if( m & S_IXOTH )
	    printf("x");
	else
	    printf("-");
    }

    printf(" ");
}

int do_ls_file(struct stat s, const char *file, const char *fname) {
    struct passwd *pwd;
    struct group  *grp;
    char	  *dt;

    if( flag(OPT_l) ) {
	printf(" %-3ld", s.st_nlink);

	if ((pwd = getpwuid(s.st_uid)) != NULL)
	    printf(" %-8.8s", pwd->pw_name);
	else
	    printf(" %-8d", s.st_uid);

	if ((grp = getgrgid(s.st_gid)) != NULL)
	    printf(" %-8.8s", grp->gr_name);
	else
	    printf(" %-8d", s.st_gid);

	dt = ctime(& s.st_mtime),
	dt[strlen(dt)-1] = '\0';

	printf(" %8lld %s",
	    (unsigned long long) s.st_size,
	    dt
	);
    }

    printf(" %s\n", fname);

    return 0;
}

int do_ls_dir(struct stat ds, const char *dir, const char *dname) {
    DIR			*opndir;
    struct dirent	*de;
    struct stat		s;
    char		nome[NAME_MAX];
    int			noff = strlen(dname);
    int			ret = 0;

    if( noff && noff < NAME_MAX ) {
	printf("%s:\n", dname);
	strncpy(nome, dname, NAME_MAX);
	nome[noff] = '/';
	++noff;
    }

    opndir = opendir(dir);
    if( opndir == (DIR *) NULL ) {
	if( noff )
	    perror(dname);
	else
	    perror(dir);

	return 1;
    }
    while( (de = readdir(opndir)) != (struct dirent *) NULL ) {
	if( ! flag(OPT_a) &&
	    ( strcmp(".",  de->d_name) == 0 ||
	      strcmp("..", de->d_name) == 0
	    )
	)
	    continue;

	strncpy(nome + noff, de->d_name, NAME_MAX - noff);

	if( stat(nome, & s) ) {
	    perror(nome);
	    ret = 1;
	}

	if( flag(OPT_l) )
	    print_mode(s.st_mode);

	if( do_ls_file(s, nome, nome) )
	    ret = 1;
    }
    closedir(opndir);

    return ret;
}

int do_ls(const char *arg, const char *aname) {
    struct stat s;
    int     ret = 0;

    if( stat(arg, & s) ) {
	perror(aname);
	return 1;
    }

    switch( s.st_mode & S_IFMT ) {
    case S_IFDIR:
	ret = do_ls_dir(s, arg, aname);
	// ret = do_ls_file(s, arg, aname);
	break;

    case S_IFIFO:
    case S_IFLNK:
    case S_IFSOCK:
    case S_IFBLK:
    case S_IFCHR:
    case S_IFREG:
    default:
	if( flag(OPT_l) )
	    print_mode(s.st_mode);
	ret = do_ls_file(s, arg, aname);
    }

    return ret;
}

int main(int argc, char *argv[]) {
    int i, ret;

    while ((i = getopt(argc, argv, "la")) != -1) {
	switch (i) {
	case 'l':
	    flags |= OPT_l;
	    break;
	case 'a':
	    flags |= OPT_a;
	    break;
	default: /* '?' */
	    fprintf(stderr, "Usage: %s [-l] [-a] [name]\n",
		    argv[0]);
	    return EXIT_FAILURE;
	}
    }

#ifdef DEBUGGING
#if DEBUGGING
    printf("flags=%x; optind=%d\n", flags, optind);
    printf("-l: %s; -a: %s\n",
	flag_ativo(flags, OPT_l),
	flag_ativo(flags, OPT_a)
    );
    printf("argc: %d\n", argc);
#endif // DEBUGGING
#endif // def DEBUGGING

    ret = EXIT_SUCCESS;
    if( optind >= argc ) {
	if( do_ls(".", "") )
	    ret = EXIT_FAILURE;
    } else {
	for( i = optind; i < argc; i++ ) {
	    if( do_ls(argv[i], argv[i]) )
		ret = EXIT_FAILURE;
	}
    }

    return ret;
}
