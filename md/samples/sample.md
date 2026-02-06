# Markdown Rendering Test Document

## 1. Paragraphs and Soft Line Breaks

This is a single paragraph
written on multiple lines
that should be rendered
as one flowing paragraph.

This is a new paragraph.
It starts after a blank line.

---

## 2. Hard Line Breaks

### 2.1 Two spaces + newline
Line one with hard break  
Line two should be on a new line.

### 2.2 Explicit HTML break
Line one<br>
Line two

---

## 3. Lists

### Unordered
- Item one
- Item two
	- Nested item
	- Another nested item
- Item three

### Ordered
1. First
2. Second
3. Third

---

## 4. Inline Formatting

- *Italic*
- **Bold**
- ***Bold Italic***
- `inline code`
- ~~Strikethrough~~

---

## 5. Code Blocks

### Fenced code (C)
```c
#include <stdio.h>

int main(void)
{
	printf("Hello, world\n");
	return 0;
}
```

### Indented code
	int x = 42;
	int y = x * 2;

---

## 6. Blockquote

> This is a blockquote.
> It spans multiple lines
> but remains a single block.

---

## 7. Tables

| Column A | Column B | Column C |
|--------:|:--------:|:---------|
| right   | center   | left     |
| 123    | 456      | 789      |

---

## 8. Links and Images

- Link: [Pandoc](https://pandoc.org)
- Inline URL: https://example.com

---

## 9. Escaping and Special Characters

Asterisk: \*  
Underscore: \_  
Backtick: \`

---

## 10. Unicode

- UTF-8 text: Привет, 世界, café
- Symbols: → ← ⇄ ✓ ✗

---

## 11. Long Wrapped Paragraph (Diff Test)

Lorem ipsum dolor sit amet, consectetur adipiscing elit,
sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris
nisi ut aliquip ex ea commodo consequat.
Это пример сноски.[^note]
Lorem ipsum dolor sit amet, consectetur adipiscing elit,
---

Lorem ipsum dolor sit amet, consectetur adipiscing elit,
[^note]: Текст сноски, который будет маленьким в DOCX.

---

123

---

Lorem ipsum dolor sit amet, consectetur adipiscing elit,  

---

LAST LINE BEFORE PAGE BREAK  
<!-- PAGE BREAK -->
FIRST  LINE AFTER PAGE BREAK  

- *Italic*
- **Bold**
- ***Bold Italic***
- `inline code`
- ~~Strikethrough~~


---


Lorem ipsum dolor sit amet, consectetur adipiscing elit,  

---


## 12. Metadata (Pandoc-compatible)

---
title: "Markdown Test (Bottom line)"
author: "Alex"
date: 2026-02-03
---

End of document.

GIT Rev.: ${GIT_BRANCH} ${GIT_REV}${GIT_MODIFIED} at ${GIT_DATE}
Converted by ${LOGNAME} at ${DATE} on ${HOSTNAME}

This document is called $TITLE and was written by $AUTHOR on $DATE.
Git rev.: $GITREV


This document is called {{title}} and was written by {{author}} on {{date}}.
Git rev.: {{gitrev}}

```
pandoc sample.md -o .output.rtf --standalone --reference-doc=reference.rtf  -M date="$(date '+%Y-%m-%d')" -M gitrev="$GIT_REV_STR --lua-filter=replace-meta.lua
```

