-- replace-meta.lua
function Pandoc(doc)
  local meta = doc.meta  -- метаданные из YAML
  for i, block in ipairs(doc.blocks) do
    if block.t == "Para" then
      local inlines = block.c
      for j, inline in ipairs(inlines) do
        if inline.t == "Str" then
          inline.text = inline.text
            :gsub("{{title}}", meta.title and meta.title[1].text or "")
            :gsub("{{author}}", meta.author and meta.author[1].text or "")
            :gsub("{{date}}", meta.date and meta.date[1].text or "")
        end
      end
    end
  end
  return doc
end
