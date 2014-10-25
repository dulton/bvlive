#!/bin/bash
for file in	`find . -name "*.c" `; do
indent -npro -nip -nlp -npsl -i4 -ts4 -sob -l100 -kr -ss -bli 0  $file
done

