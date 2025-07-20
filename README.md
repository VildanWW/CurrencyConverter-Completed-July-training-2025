# CurrencyConverterQt v0.1 — Completed

**Status:** Learning / demo project.  
**Stack:** C++17, Qt Widgets + Qt Charts, libcurl (HTTP), JSON parse.

## What’s Done
- Fetch latest FX rates (base: RUB) from REST API.
- Auto‑populate *From* / *To* currency combo boxes.
- Convert entered amount (press Enter).
- Fetch 30‑day historical rates from frankfurter.app.
- Two side‑by‑side line charts (From→EUR, To→EUR).
- Auto refresh every 5 min (QTimer).
- Graceful “no data” fallback (placeholder chart, no crash).

## Why Am I Sharing?
- Track my C++ / GUI progress (2nd‑year student).
- Show how to bind remote data to a Qt UI + charts.
- May help others starting with REST + Qt visualizations.

## Use in Production?
❌ Not without error handling, caching, config & retries.  
✅ Fine as a learning sample / prototype / student demo.

## Disclaimer
Data pulled from public demo APIs; no accuracy guarantee. Code experimental; expect rough edges.
<img width="1136" height="905" alt="erfvd" src="https://github.com/user-attachments/assets/ad3c5e4c-3546-4735-a3ee-f3c64591794e" />
<img width="1747" height="856" alt="Снимок экрана 2025-07-20 231545" src="https://github.com/user-attachments/assets/2b798adb-f78a-4e8f-b018-19d195fa970b" />
