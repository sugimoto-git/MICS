#!/usr/bin/gnuplot

# CSVファイルの設定
set datafile separator ","          # CSVの区切り文字
set key left top                    # 凡例の位置
set style data linespoints          # 折れ線+プロット
set grid                            # グリッド描画

# 各データセットの個別ファイル
file_t10i4 = "results_t10i4.csv"
file_kosarak = "results_kosarak.csv"

# ----------------------------------------------------------------------------
# 1) Pass1Time vs MinSup
# ----------------------------------------------------------------------------
set terminal pngcairo size 800,600
set output "./photos/pass1time.png"

set title "Pass1Time vs MinSup"
set xlabel "MinSup"
set ylabel "Pass1Time (sec)"

plot file_t10i4 using 2:5 title "T10I4D100K" with linespoints, \
     file_kosarak using 2:5 title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 2) Pass2Time vs MinSup
# ----------------------------------------------------------------------------
set output "./photos/pass2time.png"
set title "Pass2Time vs MinSup"
set ylabel "Pass2Time (sec)"

plot file_t10i4 using 2:6 title "T10I4D100K" with linespoints, \
     file_kosarak using 2:6 title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 3) Pass3Time vs MinSup
# ----------------------------------------------------------------------------
set output "./photos/pass3time.png"
set title "Pass3Time vs MinSup"
set ylabel "Pass3Time (sec)"

plot file_t10i4 using 2:7 title "T10I4D100K" with linespoints, \
     file_kosarak using 2:7 title "kosarak" with linespoints

# ----------------------------------------------------------------------------
# 4) GeneratedRules vs MinSup
# ----------------------------------------------------------------------------
set output "./photos/rules.png"
set title "GeneratedRules vs MinSup"
set ylabel "GeneratedRules"

plot file_t10i4 using 2:13 title "T10I4D100K" with linespoints, \
     file_kosarak using 2:13 title "kosarak" with linespoints

# 終了
set output
