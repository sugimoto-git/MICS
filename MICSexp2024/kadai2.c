#include <stdio.h>
#include <stdlib.h>

#define BUCKET_SIZE 1000  // ハッシュ表のサイズ

double MIN_SUPPORT_RATIO;  // 最小支持度を割合で指定

// アイテムペアを保持する構造体の定義
struct pair {
    int item1;          // アイテム1
    int item2;          // アイテム2 
    int count;          // 頻度
    struct pair *next;  // 次のペアへのポインタ
};

static struct pair *htab[BUCKET_SIZE];  // ハッシュ表

// ハッシュ表の初期化
void initHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        htab[i] = NULL;
    }
}

// ハッシュ値の計算（2つのアイテムの組み合わせ用）
int hash(int item1, int item2) {
    return ((item1 * 31) + item2) % BUCKET_SIZE;  // 31を乗じて分布を改善
}

// ペアの検索
struct pair *searchPair(int item1, int item2) {
    int h = hash(item1, item2);
    struct pair *p = htab[h];
    while (p != NULL) {
        if (p->item1 == item1 && p->item2 == item2) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// 新しいペアの作成
struct pair *newPair(int item1, int item2) {
    struct pair *p = malloc(sizeof(struct pair));
    if (p == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    p->item1 = item1;
    p->item2 = item2;
    p->count = 1;
    p->next = NULL;
    return p;
}

// ペアの挿入または更新
void insertOrUpdatePair(int item1, int item2) {
    struct pair *p = searchPair(item1, item2);
    if (p != NULL) {
        p->count++;
    } else {
        int h = hash(item1, item2);
        p = newPair(item1, item2);
        p->next = htab[h];
        htab[h] = p;
    }
}

// L1から候補アイテムセットC2を生成
void generateC2FromL1(const char *l1_filename) {
    FILE *fp = fopen(l1_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", l1_filename);
        exit(1);
    }

    // L1のアイテムを配列に格納
    int items[1000];
    int item_count = 0;
    int item, count;
    double support;

    while (fscanf(fp, "%d %d %lf", &item, &count, &support) != EOF) {
        items[item_count++] = item;
    }
    fclose(fp);

    // 組み合わせを生成してC2に追加
    for (int i = 0; i < item_count; i++) {
        for (int j = i + 1; j < item_count; j++) {
            insertOrUpdatePair(items[i], items[j]);
        }
    }
}

// 頻出アイテムペアの出力とファイルへの保存
void outputFrequentPairs(int transaction_count) {
    // 結果をファイルに保存
    FILE *fp = fopen("path2_result.dat", "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot create path2_result.dat\n");
        exit(1);
    }

    printf("\nFrequent item pairs (L2):\n");
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pair *p = htab[i];
        while (p != NULL) {
            double support = (double)p->count / transaction_count;
            if (support >= MIN_SUPPORT_RATIO) {
                printf("Items: (%d,%d), Count: %d, Support: %.2f%%\n",
                       p->item1, p->item2, p->count, support * 100);
                // ファイルに保存
                fprintf(fp, "%d %d %d %.2f\n", 
                       p->item1, p->item2, p->count, support * 100);
            }
            p = p->next;
        }
    }
    fclose(fp);
}

// メモリの解放
void freeHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pair *p = htab[i];
        while (p != NULL) {
            struct pair *next = p->next;
            free(p);
            p = next;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <transaction_file> <min_support_ratio>\n", argv[0]);
        return 1;


    }

    MIN_SUPPORT_RATIO = atof(argv[2]);
    initHashTab();

    // L1から候補アイテムセットC2を生成
    generateC2FromL1("path1_result.dat");

    // トランザクションデータを読み込んでC2の頻度をカウント
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", argv[1]);
        return 1;
    }

    int transaction_count = 0;
    int items[100];
    int item_count = 0;
    int tlen;

    while (fscanf(fp, "%d", &tlen) != EOF) {
        if (tlen == -1) break;
        
        // トランザクションの読み込み
        item_count = 0;
        for (int i = 0; i < tlen; i++) {
            int item;
            fscanf(fp, "%d", &item);
            items[item_count++] = item;
        }

        // トランザクション内のアイテムペアの頻度をカウント
        for (int i = 0; i < item_count; i++) {
            for (int j = i + 1; j < item_count; j++) {
                struct pair *p = searchPair(items[i], items[j]);
                if (p != NULL) {
                    p->count++;
                }
            }
        }
        
        transaction_count++;
    }
    fclose(fp);

    printf("Total transactions processed: %d\n", transaction_count);
    outputFrequentPairs(transaction_count);
    freeHashTab();

    return 0;
}

