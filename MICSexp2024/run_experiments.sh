#!/bin/bash

# 実験結果を保存するフォルダ
mkdir -p result

# ----------------------------------
# 1) 最小支持度(minsup)を変えて実験
#    最小確信度(minconf)=0.6 で固定
# ----------------------------------
for sup in 0.5 0.3 0.1 0.05 0.01 0.001
do
    # 1-1) expT10I4D100K.dat で実行
    ./kadai4 expT10I4D100K.dat $sup 0.6 \
      > result/T10I4_msup${sup}_mconf0.6.txt

    # 1-2) expkosarak.dat で実行
    ./kadai4 expkosarak.dat $sup 0.6 \
      > result/kosarak_msup${sup}_mconf0.6.txt
done

# ----------------------------------
# 2) 最小確信度(minconf)を変えて実験
#    最小支持度(minsup)=0.1 で固定
# ----------------------------------
for conf in 0.5 0.7 0.9
do
    # 2-1) expT10I4D100K.dat
    ./kadai4 expT10I4D100K.dat 0.1 $conf \
      > result/T10I4_msup0.1_mconf${conf}.txt

    # 2-2) expkosarak.dat
    ./kadai4 expkosarak.dat 0.1 $conf \
      > result/kosarak_msup0.1_mconf${conf}.txt
done

# ----
# 必要なら、ハッシュサイズを変えたバージョンの実行もここで追記可能
#  (例: kadai4_50k, kadai4_100k など別々にコンパイルしたバイナリを呼ぶ)
# ----

echo "All experiments finished. Results are in the 'result' directory."
