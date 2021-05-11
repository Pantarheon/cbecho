# CBECHO

``` /bin/sh
C:\> cbecho --help
CBECHO version 1.0
Copyright 2021 G. Adam Stanislav
All rights reserved

Usage: cbecho [options] [file]

	cbecho -# (strips off # number characters)
	cbecho -U (tries Unicode first)*
	cbecho -a [-o] file (will save file in ANSI)
	cbecho -b (bypasses Unicode)
	cbecho -h (returns this message)
	cbecho -e (empties the clipboard after output)
	cbecho -k (does not empty the clipboard)*
	cbecho -o file (sends the output to the file)
	cbecho -u [-o] file (may save in Unicode)*
	cbecho -v (prints the version)
	cbecho -w (wipes the clipboard)

	* marks the default setting.
	If the last argument is just -o, the output is stdout.
	Because this is Windows, you can use / instead of -.
C:\>
```

## GUI vs. CLI

There are two main types of computer programs. One interacts with a user
through a _graphical user interface_ (`GUI`). The other gets its input
through a standard input (`stdin`) and sends its output to a standard output
(`stdout`), generally in the form of some text through a _command line
interface_ (`CLI`).

In `CLI`, the standard input can come from a computer keyboard, or a file, or
a device, or some other source. The program neither knows nor cares where it
comes from. Similarly, the standard output can go to a monitor, a printer,
a file, a device, wherever. Again, the program neither knows nor cares where
the output goes.

### Bridging GUI and CLI

Microsoft Windows comes with a `CLI` utility called `clip`, which accepts its
input from `stdin` and sends its output as plain text to the Windows clipboard.
That text can then be pasted to `GUI` programs, thus bridging one half of the
gap between `GUI` and `CLI` programs. For example,

```
C:\> dir | clip
```

This will copy the directory listing of the `C:\` folder to the clipboard, from
where it can be pasted to a text editor, or an HTML editor, or many other `GUI`
programs.

Recently, I needed to move in the opposite direction, i.e., to copy a portion
of text from a text editor and somehow send it to the standard input of a `CLI`
program, which processed the text and sent it to its standard output.

At first I would copy and paste that text to a temp file. I then redirected
that file to a chain of small `CLI` programs. Something like this:

```
C:\> program1 < temp.txt | program2 | program3 | clip
```

Then I could paste the result to my `GUI` editor.

Doing so once is no big deal. Doing so repeatedly is tedious. It is also easy
to make mistakes. So to bridge the other half of the gap between `CLI` and
`GUI`, I wrote `cbecho`, which stands for `clipboard echo`. Now I can just
copy the text, but instead of using a temp file, I type something like this,

```
C:\> cbecho | program1 | program2 | program3 | clip
```

Much simpler and much faster!

Between `clip` and `cbecho`, the bridge between `GUI` and `CLI` software
is now complete.

> __A tip:__ If you wanted to copy the output of the `dir` command to
> the clipboard, but still see it in your command line window, try this,
>
> ```
> C:\> dir | clip & cbecho
> ```

## Using cbecho

In most cases, all you need to do is just type `cbecho` on the command line,
and if there is any plain text, whether Unicode or ANSI (or other local text),
on the clipboard, `cbecho` will send it to `stdout`, from where you can pipe
it to other programs, or redirect it to a device, etc.

It does, however, offer various options. To see them, just type any of the
following,

```
C:\> cbecho -h
C:\> cbecho /?
C:\> cbecho --help
```

That will list all of its command line switches, just as shown above. If you
have the `CBECHO` environmental variable set, it will list that as well.

If you just need to know what version of `cbecho` you have, you can type one
of these two,

```
C:\> cbecho -v
C:\> cbecho --version
```

### The options

By default, `cbecho` will print the text just as it is stored on the clipboard.
Some Windows programs append an extra carriage return and line feed to the text
before copying it to the clipboard. You can strip them with this option,

```
C:\> cbecho -2
```

Note that the `-` just denotes a command line switch, not the minus sign.
So this would give you the same result,

```
C:\> cbecho /2
```

Indeed in any of the options that start with a single `-` you can use a `/`
instead. Except that I noticed the slash does not work in `git-bash` which
apparently assumes it is denoting a path. That is why I always use the dash
instead.

You can strip any number (theoretically up to 2,147,483,647) of trailing
characters by typing the number (without any commas, so up to 2147483647) as a
switch. For example, this strips the last 50 characters (and will show nothing
if there are only 50 or fewer characters of text on the clipboard),

```
C:\> cbecho -50
```

By the way, if you use this switch more than once, only the last occurrence
will count (this is true with any switches that may ask for contradicting
actions). So you can know how many characters will be stripped if you type,

```
C:\> cbecho -17 -b -2147483647 -U -0
```

None of them will be stripped because the zero switch appears after the
other numeric switches (also the `-U` cancels out the `-b`, as shown next).

Windows allows programs to copy all kinds of data to the clipboard, not just
plain text. Even with the plain text, it can be Unicode text, ANSI text, or
just text. By default, `cbecho` first checks if any Unicode text is on the
clipboard. Only if it does not find Unicode, will it check for ANSI text and
just text.

You can instruct `cbecho` to bypass the Unicode text and go straight to looking
for ANSI text or just text like this,

```
C:\> cbecho -b 
```

And you can tell it to search for Unicode text first after all like this,

```
C:\> cbecho -U
```

That is the _capital_ leter `U`.

By default, `cbecho` writes its output to `stdout`. You can instruct it to
write to a file. Assuming the file name is called `myfile.txt`, any of these
will tell `cbecho` to write to it,

```
C:\> cbecho -o myfile.txt
C:\> cbecho -omyfile.txt
C:\> cbecho myfile.txt
```

The `-o` switch is optional there. If, however, `-o` is the _last_ switch on
the command line, `cbecho` will send this output to `stdout`. But only if it
is the last. Otherwise it assumes it is followed by a file name, even if it
starts with a `-`, which is a perfectly valid character in a file name.

That is, indeed, why we even have the `-o` switch. While you can normally skip
it and just type the file name, if you do so with a file whose name starts
with a dash, `cbecho` will assume it is a switch. But placing it after the `-o`
switch tells `cbecho` to treat it as a file name. For example, consider these
four possibilities

```
C:\> cbecho -Xantipa.txt
C:\> cbecho -o -Xantipa.txt
C:\> cbecho -Universe.txt
C:\> cbecho -o -Universe.txt
```

The first one will make `cbecho` assume it is meant to be the `-X` switch,
so it will consider it an error. In the second case, it will send its output
to `-Xantipa.txt`. In the third case, it will assume you meant `-U`, so it
will read the Unicode text from the clipboard and write it to `stdout`. In
the fourth case, it will send its output to the file called `-Universe.txt`.

> __A tip:__ As long as you do not tell `cbecho` to empty the clipboard, you
> can chain the command to send its output to several files. Since `cbecho`
> returns `0` (which in command shells means `no error`), chaining with `&&`
> will run the whole chain if there is plain text on the clipboard, but will
> quit after the first `cbecho` exits `1`, meaning there is no plain text on the
> clipboard. For example,
>
> ```
> C:\> cbecho file1.txt && cbecho file2.txt && cbecho -e
> ```
>
> If there is plain text on the clipboard, this will write it to `file1.txt`,
> then to `file2.txt`, then send it to `stdout` and finally empty the clipboard
> (because of the `-e` switch described somewhat below).

Unlike other operating systems, Windows can treat files as either binary or
text files. I noticed that if I send the output of `cbecho` to a text file,
it is a plain non-Unicode file even if the text comes from the clipboard in
Unicode.

If, on the other hand, I send it to a binary file, the Unicode text remains in
16-bit Unicode, while the ANSI text remains the ANSI text.

By default, `cbecho` will treat the file as binary, so Unicode can remain
Unicode (hopefully). This can also be requested explicitly,

```
C:\> cbecho -u filename.txt
```

But you can also request it to be treated as a text file, so Unicode is saved
as ANSI (again, hopefully),

```
C:\> cbecho -a filename.txt
```

Last but not least, by default `cbecho` leaves the data on the clipboard
even after reading it. But you can ask it to empty the clipboard after it
displayed the text (assuming there was some text on the clipboard). You can
also ask it to wipe the clipboard without even reading it. That is useful when
some program copied something to the clipboard and left it there long after
it is needed. Removing the data from the clipboard also frees whatever memory
was used to hold that data.

These three options are,

```
C:\> cbecho -k
C:\> cbecho -e
C:\> cbecho -w
```

The `-k` option tells `cbecho` to `keep` the data on the clipboard. The `-e`
option will `empty` the clipboard after sending any text from it to `stdout`.
And the `-w` option will `wipe` the clipboard without any output.

Both `-e` and `-w` will leave the clipboard equally empty. The only difference
is that `-e` will first check if there is some plain text on the clipboard,
and if so, it will output it.

> __A tip:__ The `-e` option will empty the clipboard after first checking
> for text so it can display it if it is present. But it will still empty the
> clipboard even if it does not find any text on it.
>
> What if you wanted to empty the clipboard if and only if there is text on it?
> Try,
>
> ```
> C:\> cbecho && cbecho -w
> ```
>
> That will check for the clipboard text. If it finds any, it will display it
> and exit `0`, so the command shell will continue on to the `cbecho -w`, which
> will wipe the clipboard clean.
>
> And what if you wanted to wipe the clipboard only if there is text on it, but
> without `cbecho` displaying the text? Try,
>
> ```
> C:\> cbecho NUL && cbecho -w
> ```


### Preferences

You can override any of the defaults with the command line parameters as
described above. You can also override them permanently or semipermanently
by setting the `CBECHO` environment variable to any of the above options
(note that `-h` and `-v` are not options, they are requests for help or
version, so adding them to the environment variable has no effect).

You cannot specify a file name in the environment variable, only the options.
So, the `o` option will only affirm `stdout` as the output and is, therefore,
redundant, though you can use it if you want.

There is no need to prepend the options in the environmental variable with
a dash. For example, this will result in bypassing Unicode, stripping two
characters from the end of the text and emptying the clipboard,

```
set CBECHO=b2e
```

The `cbecho` program checks the environment variable first, the command line
parameters next. And since conflicting parameters are resolved by the last one
standing, you can overturn the preferences set by the environment variable
with command line parameters.

The name of the variable is not case sensitive, but the parameters are (that is
how we can have `-u` and `-U` meaning different things).

## Installation

This is a very simple program entirely contained in the small `cbecho.exe`
file. No need to install it, no need to uninstall it. Just download it and
place it in a folder listed in the `PATH` environmental variable. Then you
can use it from the command line as well as from batch files.

## License

```
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
```