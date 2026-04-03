# Underverse - Markdown Help
This markdown syntax documentation is not complete.
A complete documentation can be found here.

## Paragraphs
Paragraphs are blocks of text separated by one or more empty line.  
To force a linebreak, end a line within a paragraph with two space characters.

## Highlighting
Highlighting text by making it bold or italic is done like this:  
This text is *itallic*, this text is **bold**!

## Hyperlinks
Links are created like this:  
[Google](https://www.google.de).  
If the link text and link target are the same, you can use this short form:  
<http://www.google.de>

## Headers
Headers are created by prepending hash (#) character to a line.
The number of hash characters determines the header levels (1-6):

# First level header
some text

## Second level header
some more text

### Third level header
even more text

## Source Code
In-line code is highlighted with back-ticks, e.g. see the `print()` function.

Multi-line code is framed by triple backticks:  
```
line one
line two
line three
```

## Lists
Unordered bullet-point lists are create by prepending lines with `-` characters:
- bla  
- bla
- bla

Ordered lists use numbers with periods.
Indentation of sub-items can be done with tabs:
1. first
2. second
3. third
    1. sub 1
	2. sub 2

## Images
Images are inserted like that:  
![alt text]([image path])  
alt text is displayed when the image cannot be displayed, i.e. when it is not present.
           
## Tables
Tables are created like that:

| Month    | Savings |
| :------- | ------: |
| January  | $250    |
| February | $80     |
| March    | $420    |

## Horizontal lines
Horizontal lines are created by putting three asterisks in a line:
***
