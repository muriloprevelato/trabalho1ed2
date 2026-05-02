#!/bin/bash
ERROS=0
for cmd in \
    "c1.geo c1.pm c1/censo.qry" \
    "c1.geo c1.pm c1/despj-001-hab.qry" \
    "c1.geo c1.pm c1/despj-001.qry" \
    "c1.geo c1.pm c1/despj-010.qry" \
    "c1.geo c1.pm c1/despj-100-hab.qry" \
    "c1.geo c1.pm c1/despj-100.qry" \
    "c1.geo c1.pm c1/falecimento-unico-morador.qry" \
    "c1.geo c1.pm c1/falecimento-unico-st.qry" \
    "c2.geo c2.pm c2/censo.qry" \
    "c3.geo c3.pm c3/censo.qry"
do
    read geo pm qry <<< $cmd
    rm -f saida/*
    RESULT=$(valgrind --leak-check=full ./src/ted -e testes -f $geo -pm $pm -q $qry -o saida 2>&1 | grep "ERROR SUMMARY")
    echo "$geo / $qry → $RESULT"
done
