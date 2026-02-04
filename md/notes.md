

# Markdown  commands

```
pandoc sample.md -o sample.rtf --standalone --wrap=preserve

```

## Reference using:
```
pandoc /dev/null -t rtf -o reference.rtf --standalone
```
```
pandoc sample.md \
	-o sample.rtf \
	--standalone \
	--wrap=preserve \
	--reference-doc=reference.rtf

pandoc sample.md -o sample.rtf --standalone --reference-doc=reference.rtf
pandoc sample.md -o output.rtf --standalone --reference-doc=reference.rtf -M date="$(date '+%Y-%m-%d')"
pandoc sample.md -o output.rtf --standalone --reference-doc=reference.rtf --lua-filter=replace-meta.lua

```

| Markdown     | RTF стиль         |
| ------------ | ----------------- |
| Body text    | Normal            |
| `# Heading`  | Heading 1         |
| `## Heading` | Heading 2         |
| Code block   | Preformatted Text |
| Blockquote   | Block Text        |
| Lists        | List styles       |
| Table        | Table styles      |


Если в начале Markdown есть YAML metadata block:
```
---
title: "Markdown Test"
author: "Alex"
date: 2026-02-03
---
```
То при использовании --standalone Pandoc автоматически вставит их в “title block”:

Добавление даты через опцию -M
pandoc sample.md -o output.rtf --standalone -M date="$(date '+%Y-%m-%d')"


В Title block в начале документа Pandoc заменит дату

В теле документа без шаблона — подстановки нет

--template работает только с .docx, .tex, .html, .odt (т.е. текстовыми шаблонами).

```
pandoc sample.md -o output.rtf --standalone  -M date="$(date '+%Y-%m-%d')"
pandoc sample.md -o .output.rtf --standalone --reference-doc=reference.rtf  -M date="$(date '+%Y-%m-%d')" -M gitrev="$GIT_REV_STR --lua-filter=replace-meta.lua
```
