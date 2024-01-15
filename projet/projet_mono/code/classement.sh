#!/bin/bash

fichierSource="classement.tex"
fichierSortie="classement.pdf"

echo "\\documentclass{article}" > "$fichierSource"
echo "\\begin{document}" >> "$fichierSource"
echo "Classement (le tri des scores pas encore implémenté)." >> "$fichierSource"
echo "\\end{document}" >> "$fichierSource"

pdflatex -interaction=batchmode "$fichierSource"

rm -f "$fichierSource" classement.aux classement.log
mv "$fichierSource" "$fichierSortie"
mv "$fichierSortie" ../logs/stats/
echo "Le fichier PDF \"$fichierSortie\" a été créé avec succès."
