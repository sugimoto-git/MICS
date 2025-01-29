#!/usr/bin/gnuplot

# CSVファイルの区切りをカンマに指定
set datafile separator ","

# グラフスタイルなど
set style data linespoints
set grid
set key left top

# 入力CSVファイル名を変数に
resultsFile = "results.csv"

# 出力フォーマットはPNGに設定 (サイズ調整はお好みで)
set terminal pngcairo size 800,600

# ----------------------------------------------------------------------------
# 1) Pass1Time vs MinSup (datasetごとに別線)
# ----------------------------------------------------------------------------
set output "pass1time.png"
set title "Pass1Time vs MinSup"
set xlabel "MinSup"
set ylabel "Pass1Time (sec)"

plot \
    resultsFile using (strcmp(strcol(1),"expT10I4D100K.dat")==0 ? column(2) : 1/0):5 \
        title "T10I4D100K" with linespoints, \
    resultsFile using (strcmp(strcol(1),"expkosarak.dat")==0 ? column(2) : 1/0):5 \
        title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 2) Pass2Time vs MinSup
# ----------------------------------------------------------------------------
set output "pass2time.png"
set title "Pass2Time vs MinSup"
set ylabel "Pass2Time (sec)"

plot \
    resultsFile using (strcmp(strcol(1),"expT10I4D100K.dat")==0 ? column(2) : 1/0):6 \
        title "T10I4D100K" with linespoints, \
    resultsFile using (strcmp(strcol(1),"expkosarak.dat")==0 ? column(2) : 1/0):6 \
        title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 3) Pass3Time vs MinSup
# ----------------------------------------------------------------------------
set output "pass3time.png"
set title "Pass3Time vs MinSup"
set ylabel "Pass3Time (sec)"

plot \
    resultsFile using (strcmp(strcol(1),"expT10I4D100K.dat")==0 ? column(2) : 1/0):7 \
        title "T10I4D100K" with linespoints, \
    resultsFile using (strcmp(strcol(1),"expkosarak.dat")==0 ? column(2) : 1/0):7 \
        title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 4) GeneratedRules vs MinSup
# ----------------------------------------------------------------------------
set output "rules.png"
set title "GeneratedRules vs MinSup"
set ylabel "GeneratedRules"

plot \
    resultsFile using (strcmp(strcol(1),"expT10I4D100K.dat")==0 ? column(2) : 1/0):13 \
        title "T10I4D100K" with linespoints, \
    resultsFile using (strcmp(strcol(1),"expkosarak.dat")==0 ? column(2) : 1/0):13 \
        title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 終了
# ----------------------------------------------------------------------------
set output
