#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

/*****************************************************************************
	Copyright 2021, G. Adam Stanislav.
	All rights reserved.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

******************************************************************************

	Link with User32.lib (the pragma below should take care of it).

*****************************************************************************/

#pragma comment(lib,"user32")
#define	ENVPREFS	"CBECHO="

int version(int i) {
	fprintf(i ? stderr : stdout, "CBECHO version 1.0\n"
		"Copyright 2021 G. Adam Stanislav\nAll rights reserved\n");
	return i;
}

int usage(int i, char *prefs) {
	version(i);
	fprintf(i ? stderr : stdout, "\nUsage: cbecho [options] [file]\n\n"
		"\tcbecho -# (strips off # number characters)\n"
		"\tcbecho -U (tries Unicode first)*\n"
		"\tcbecho -a [-o] file (will save file in ANSI)\n"
		"\tcbecho -b (bypasses Unicode)\n"
		"\tcbecho -h (returns this message)\n"
		"\tcbecho -e (empties the clipboard after output)\n"
		"\tcbecho -k (does not empty the clipboard)*\n"
		"\tcbecho -o file (sends the output to the file)\n"
		"\tcbecho -u [-o] file (may save in Unicode)*\n"
		"\tcbecho -v (prints the version)\n"
		"\tcbecho -w (wipes the clipboard)\n"
		"\n\t* marks the default setting."
		"\n\tIf the last argument is just -o, the output is stdout.\n"
		"\tBecause this is Windows, you can use / instead of -.\n");
	if ((prefs) && (prefs[strlen(ENVPREFS)]))
		fprintf(i ? stderr : stdout, "\n\tYour current system preferences are set to '%s'.\n", prefs);
	return i;
}

int main(int argc, char *argv[], char **envp) {
	FILE *output;
	char *filename = NULL;
	char *mode = "wb";
	char *prefs = NULL;
	int n = 0;
	int empty = 0;
	int bypass = 0;
	int retval = 1;
	unsigned int i;

	/* Parse the environment for user defaults */
	for (;*envp;envp++) {
		if (strnicmp(ENVPREFS, *envp, strlen(ENVPREFS)) == 0) {
			char *str;
			prefs = *envp;
			for (str = *envp; *str; str++) switch (*str) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					n = atoi(str);
					while (isdigit(*str)) str++;
					break;
				case 'U':
					bypass = 0;
					break;
				case 'a':
					mode = "wt";
					break;
				case 'b':
					bypass = 1;
					break;
				case 'e':
					empty = 1;
					break;
				case 'k':
					empty = 0;
					break;
				case 'o':
					filename = NULL;
					break;
				case 'u':
					mode = "wb";
					break;
				case 'w':
					empty = -1;
					break;
			}
			break;	/* from the envp for loop */
		}
	}

	/* Parse the command line */
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help")) return usage(0, prefs);
		else if (!strcmp(argv[i], "--version")) return version(0);
		else if ((argv[i][0] == '/') || (argv[i][0] == '-')) {
			if (isdigit(argv[i][1]))
				n = atoi(&(argv[i][1]));
			else switch (argv[i][1]) {
				default:
					return usage(-2, prefs);
					break;
				case 'U':
					bypass = 0;
					break;
				case 'a':
					mode = "wt";
					if (argv[i][2]) filename = &argv[i][2];
					break;
				case 'b':
					bypass = 1;
					break;
				case 'e':
					empty = 1;
					break;
				case 'h':
				case '?':
					return usage(0, prefs);
					break;
				case 'k':
					empty = 0;
					break;
				case 'o':
					if (argv[i][2]) filename = &argv[i][2];
					else if (i < argc-1) filename = argv[++i];
					else filename = NULL;
					break;
				case 'u':
					mode = "wb";
					if (argv[i][2]) filename = &argv[i][2];
					break;
				case 'v':
					return version(0);
					break;
				case 'w':
					empty = -1;
					break;
			}
		}
		else {	/* No '-' or '/' switch, assume filename */
			filename = argv[i];
		}
	}

	if (empty < 0) filename = NULL;

	if (filename == NULL) output = stdout;
	else if ((output = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "Could not create file %s\n", filename);
		return usage(-3, prefs);
	}

	if (OpenClipboard(NULL)) {
		if (empty >= 0) {
			HANDLE hClipboardData;
			int temp;

			if ((bypass == 0) && (IsClipboardFormatAvailable(CF_UNICODETEXT))) {
				wchar_t *ws;
				hClipboardData = GetClipboardData(CF_UNICODETEXT);
				ws = (wchar_t*)GlobalLock(hClipboardData);
				if ((temp = wcslen(ws)) > n)
					fwprintf(output, L"%.*s", temp-n, ws);
				GlobalUnlock(hClipboardData);
				retval = 0;
			}
			else if (IsClipboardFormatAvailable(CF_TEXT) ||
					IsClipboardFormatAvailable(CF_OEMTEXT)) {
				char *str;
				hClipboardData = GetClipboardData(CF_TEXT);
				str = (char*)GlobalLock(hClipboardData);
				if ((temp = strlen(str)) > n)
				fprintf(output, "%.*s", temp-n, str);
			retval = 0;
			}
			if (filename != NULL) fclose(output);
		}
		if ((empty != 0) && (EmptyClipboard() == 0))
			retval = -4;
		CloseClipboard();
	}
	return retval;
}