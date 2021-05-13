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
	fprintf(i ? stderr : stdout, "CBECHO version 1.0a\n"
		"Copyright 2021 G. Adam Stanislav\nAll rights reserved\n");
	return i;
}

int usage(int i, char *prefs) {
	version(i);
	fprintf(i ? stderr : stdout, "\nUsage: cbecho [options] [file]\n\n"
		"\tcbecho -# (strip off # number characters)\n"
		"\tcbecho -B (insert BE BOM)\n"
		"\tcbecho -C (do not convert Unicode to UTF-8)*\n"
		"\tcbecho -L (insert LE BOM)\n"
		"\tcbecho -O file (append the output to the file)\n"
		"\tcbecho -P (preserve BOM)*\n"
		"\tcbecho -R (remove BOM)\n"
		"\tcbecho -U (try Unicode first)*\n"
		"\tcbecho -a [-o] file (save file in ANSI)\n"
		"\tcbecho -b (bypass Unicode)\n"
		"\tcbecho -c (convert Unicode to UTF-8)\n"
		"\tcbecho -e (empty the clipboard after output)\n"
		"\tcbecho -h (return this message)\n"
		"\tcbecho -k (do not empty the clipboard)*\n"
		"\tcbecho -n (do not swap Unicode bytes)*\n"
		"\tcbecho -o file (send the output to the file)\n"
		"\tcbecho -s (swap Unicode bytes)\n"
		"\tcbecho -u [-o] file (save in Unicode if available)*\n"
		"\tcbecho -v (print the version)\n"
		"\tcbecho -w (wipe the clipboard)\n"
		"\n\t* marks the default setting."
		"\n\tIf the last argument is just -o or -O, the output is stdout.\n"
		"\tBecause this is Windows, you can use / instead of -.\n");
	if ((prefs) && (prefs[strlen(ENVPREFS)]))
		fprintf(i ? stderr : stdout, "\n\tYour current system preferences are set to '%s'.\n", prefs);
	return i;
}

void utf(wchar_t wc, int swap, FILE *output) {
	if (swap) wc = ((wc << 8) | ((wc >> 8) & 0xFF)) & 0xFFFF;
	if (wc < 0x0080)
		fprintf(output, "%c", (char)wc);
	else if (wc < 0x0800)
		fprintf(
			output, "%c%c",
			(char)(0xC0 | ((wc >> 6) & 0x1F)),
			(char)(0x80 | (wc & 0x3F))
		);
	else
		fprintf(
			output, "%c%c%c",
			(char)(0xE0 | ((wc >> 12) & 0x0F)),
			(char)(0x80 | ((wc >> 6) & 0x3F)), 
			(char)(0x80 | (wc & 0x3F))
		);
}

int main(int argc, char *argv[], char **envp) {
	FILE *output;
	char *filename = NULL;
	char *mode;
	char *prefs = NULL;
	int n = 0;
	int bin = 1;
	int	app = 0;
	int empty = 0;
	int bypass = 0;
	int retval = 1;
	int swap = 0;
	int bom = 0;
	int rbom = 0;	// remove BOM
	int convert = 0;	// to UTF-8
	unsigned int i;

	/* Parse the environment for user defaults */
	for (;*envp;envp++) {
		if (strnicmp(ENVPREFS, *envp, strlen(ENVPREFS)) == 0) {
			char *str;
			prefs = *envp;
		for (str = &(prefs[strlen(ENVPREFS)]); *str; str++) switch (*str) {
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
				case 'B':
					bom  = 1;
					rbom = 1;
					break;
				case 'C':
					convert = 0;
					break;
				case 'L':
					bom  = -1;
					rbom =  1;
					break;
				case 'O':
					app = 1;
					break;
				case 'P':
					bom  = 0;
					rbom = 0;
					break;
				case 'R':
					bom  = 0;
					rbom = 1;
					break;
				case 'U':
					bypass = 0;
					break;
				case 'a':
					bin = 0;
					break;
				case 'b':
					bypass = 1;
					break;
				case 'c':
					convert = 1;
					break;
				case 'e':
					empty = 1;
					break;
				case 'k':
					empty = 0;
					break;
				case 'n':
					swap = 0;
					break;
				case 'o':
					app = 0;
					break;
				case 's':
					swap = 1;
				case 'u':
					bin = 1;
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
				case 'B':
					bom  = 1;
					rbom = 1;
					break;
				case 'C':
					convert = 0;
					break;
				case 'L':
					bom  = -1;
					rbom =  1;
					break;
				case 'O':
					app = 1;
					if (argv[i][2]) filename = &argv[i][2];
					else if (i < argc-1) filename = argv[++i];
					else filename = NULL;
					break;
				case 'P':
					bom  = 0;
					rbom = 0;
					break;
				case 'R':
					bom  = 0;
					rbom = 1;
				case 'U':
					bypass = 0;
					break;
				case 'a':
					bin = 0;
					if (argv[i][2]) filename = &argv[i][2];
					break;
				case 'b':
					bypass = 1;
					break;
				case 'c':
					convert = 1;
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
				case 'n':
					swap = 0;
					break;
				case 'o':
					app = 0;
					if (argv[i][2]) filename = &argv[i][2];
					else if (i < argc-1) filename = argv[++i];
					else filename = NULL;
					break;
				case 's':
					swap = 1;
					break;
				case 'u':
					bin = 1;
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
	else {
		if (app) mode = (bin) ? "ab" : "at";
		else mode = (bin) ? "wb" : "wt";
		if ((output = fopen(filename, mode)) == NULL) {
			fprintf(stderr, "Could not create file %s\n", filename);
			return usage(-3, prefs);
		}
	}

	if (OpenClipboard(NULL)) {
		if (empty >= 0) {
			HANDLE hClipboardData;
			int temp;

			if ((bypass == 0) && (IsClipboardFormatAvailable(CF_UNICODETEXT))) {
				wchar_t *ws;

				hClipboardData = GetClipboardData(CF_UNICODETEXT);
				ws = (wchar_t*)GlobalLock(hClipboardData);

				if (((rbom)||(convert))&&((*ws == 0xFFFE) || (*ws == 0xFEFF))) ws++;

				if ((temp = wcslen(ws)) > n) {
					temp -= n;
					/* NOTE: fputwchar does not seem to do it, so we
					         use fwprintf even for a single wchar_t */

					if (convert) {
						if (bom)
							utf((wchar_t)0xFEFF, 0, output);
						for (i = 0; i < temp; i++, ws++) {
							utf(*ws, swap, output);
						}
					}
					else {
						if (bom)
							// Windows is low-endian, so when we pass FEFF,
							// it will output FF followed by FE, and vice versa.
							fwprintf(output, L"%c",
								bom < 0 ? (wchar_t)0xfeff : (wchar_t)0xfffe);
						if (swap == 0) {
							fwprintf(output, L"%.*s", temp, ws);
						}
						else {
							wchar_t wc;
							unsigned char *str;

							for (i = 0; i < temp; i++, ws++) {
								str = (char *)ws;
								wc = (wchar_t)((str[0]<<8)|str[1]);
								fwprintf(output, L"%c", wc);
							}
						}
					}
				}
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
				GlobalUnlock(hClipboardData);
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