#!/bin/bash
i=$2+1
((i=$i+1))
cat $1 | tr -c [:alpha:] "\\n" | tr [:upper:] [:lower:]| sort | uniq -c | sort -nk 1 | tac | head -n $i | tail -n $2
