

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
