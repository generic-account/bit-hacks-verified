---
layout: default
title: Verified Bit Hacks
---

# Verified Bit Hacks

A growing reference of low-level tricks and optimizations with machine-checked proofs (Coq/Rocq).  
Use the [search]({{ '/search.html' | relative_url }}) page, browse by tags, or scan the catalog below.

## Tags
<ul>
{% assign all = "" | split: "" %}
{% for p in site.proofs %}
  {% if p.tags %}
    {% assign all = all | concat: p.tags %}
  {% endif %}
{% endfor %}
{% assign uniq = all | uniq | sort %}
{% for t in uniq %}
  <li><a href="{{ '/tags/' | relative_url }}{{ t | slugify }}/">{{ t }}</a></li>
{% endfor %}
</ul>

## Catalog
{% assign groups = site.proofs | group_by: "categories" %}
{% for g in groups %}
  {% assign header = g.name
    | replace: '["',''
    | replace: '"]',''
    | replace: '","',' / ' %}
### {{ header | default: "root" }}
<ul>
  {% assign sorted = g.items | sort: "title" %}
  {% for p in sorted %}
    <li>
      <a href="{{ p.url | relative_url }}">{{ p.title }}</a>
      {% if p.summary %} — {{ p.summary }}{% endif %}
      {% if p.tags and p.tags.size > 0 %}
        <small>[{{ p.tags | join: ", " }}]</small>
      {% endif %}
    </li>
  {% endfor %}
</ul>
{% endfor %}
