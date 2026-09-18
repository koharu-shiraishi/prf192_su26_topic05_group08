# Component manager logic notes

This note records the main logic failures fixed in the current source tree.

## CSV loading

The old loader used `sscanf` with comma scansets. That failed for:

- empty fields such as `,,`;
- quoted fields containing commas;
- names longer than the destination width;
- Windows CRLF/BOM files from spreadsheet tools.

The new loader parses CSV fields explicitly and replaces the manager only after parsing succeeds.

## IDs

The sample CSV originally contained a duplicate ID `25`. Duplicate IDs break update/delete/search-by-ID because only the first matching record is reachable. The new loader repairs duplicate or zero IDs and recalculates `next_id`.

## Numeric validation

Huge unsigned values such as `4294967229` often indicate a negative value that underflowed into a `uint32_t`. The new validation rejects values outside the business limits before records enter the manager.

## Sorting

The old merge sort used boolean comparators. The new sort uses standard three-way comparators, preventing ascending/descending inversion.

## Input handling

The new UI uses `fgets` everywhere and parses numbers explicitly. This avoids leftover-newline bugs caused by mixing `scanf` and `fgets`.
