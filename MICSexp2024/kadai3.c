#include <stdio.h>
#include <stdlib.h>

#define BUCKET_SIZE 1000  // ハッシュ表のサイズ

double MIN_SUPPORT_RATIO;  // 最小支持度を割合で指定

// 3アイテムの組み合わせを保持する構造体
struct triple {
    int item1;          // アイテム1
    int item2;          // アイテム2
    int item3;          // アイテム3
    int count;          // 頻度
    struct triple *next;  // 次の要素へのポインタ
};

static struct triple *htab[BUCKET_SIZE];  // ハッシュ表

// パス2の結果を保持する構造体
struct pair_result {
    int item1;
    int item2;
    struct pair_result *next;
};

static struct pair_result *frequent_pairs = NULL;

// ハッシュ表の初期化
void initHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        htab[i] = NULL;
    }
}

// ハッシュ値の計算（3つのアイテムの組み合わせ用）
int hash(int item1, int item2, int item3) {
    return ((item1 * 31 + item2) * 31 + item3) % BUCKET_SIZE;
}

// 3アイテムの組み合わせの検索
struct triple *searchTriple(int item1, int item2, int item3) {
    int h = hash(item1, item2, item3);
    struct triple *p = htab[h];
    while (p != NULL) {
        if (p->item1 == item1 && p->item2 == item2 && p->item3 == item3) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// 新しい3アイテムの組み合わせの作成
struct triple *newTriple(int item1, int item2, int item3) {
    struct triple *p = malloc(sizeof(struct triple));
    if (p == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    p->item1 = item1;
    p->item2 = item2;
    p->item3 = item3;
    p->count = 1;
    p->next = NULL;
    return p;
}

// 3アイテムの組み合わせの挿入または更新
void insertOrUpdateTriple(int item1, int item2, int item3) {
    // アイテムを昇順にソート
    int temp;
    if (item1 > item2) { temp = item1; item1 = item2; item2 = temp; }
    if (item2 > item3) { temp = item2; item2 = item3; item3 = temp; }
    if (item1 > item2) { temp = item1; item1 = item2; item2 = temp; }

    struct triple *p = searchTriple(item1, item2, item3);
    if (p != NULL) {
        p->count++;
    } else {
        int h = hash(item1, item2, item3);
        p = newTriple(item1, item2, item3);
        p->next = htab[h];
        htab[h] = p;
    }
}

// パス2の結果を読み込む
void loadL2(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        exit(1);
    }

    int item1, item2, count;
    double support;
    while (fscanf(fp, "%d %d %d %lf", &item1, &item2, &count, &support) != EOF) {
        struct pair_result *p = malloc(sizeof(struct pair_result));
        if (p == NULL) {
            fprintf(stderr, "Error: malloc failed\n");
            exit(1);
        }
        p->item1 = item1;
        p->item2 = item2;
        p->next = frequent_pairs;
        frequent_pairs = p;
    }
    fclose(fp);
}

// 頻出ペアから候補3アイテムセットを生成
void generateC3FromL2() {
    struct pair_result *p1 = frequent_pairs;
    while (p1 != NULL) {
        struct pair_result *p2 = p1->next;
        while (p2 != NULL) {
            // 2つのペアが1つのアイテムを共有している場合、3アイテムセットを生成
            if (p1->item2 == p2->item1) {
                insertOrUpdateTriple(p1->item1, p1->item2, p2->item2);
            }
            p2 = p2->next;
        }
        p1 = p1->next;
    }
}

// 頻出3アイテムセットの出力
void outputFrequentTriples(int transaction_count) {
    printf("\nFrequent item triples (L3):\n");
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct triple *p = htab[i];
        while (p != NULL) {
            double support = (double)p->count / transaction_count;
            if (support >= MIN_SUPPORT_RATIO) {
                printf("Items: (%d,%d,%d), Count: %d, Support: %.2f%%\n",
                       p->item1, p->item2, p->item3, p->count, support * 100);
            }
            p = p->next;
        }
    }
}

// メモリの解放
void freeHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct triple *p = htab[i];
        while (p != NULL) {
            struct triple *next = p->next;
            free(p);
            p = next;
        }
    }

    struct pair_result *p = frequent_pairs;
    while (p != NULL) {
        struct pair_result *next = p->next;
        free(p);
        p = next;
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <transaction_file> <min_support_ratio>\n", argv[0]);
        return 1;
    }

    MIN_SUPPORT_RATIO = atof(argv[2]);
    initHashTab();

    // L2から候補3アイテムセットC3を生成
    loadL2("path2_result.dat");
    generateC3FromL2();

    // トランザクションデータを読み込んでC3の頻度をカウント
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

        // トランザクション内の3アイテムの組み合わせの頻度をカウント
        for (int i = 0; i < item_count - 2; i++) {
            for (int j = i + 1; j < item_count - 1; j++) {
                for (int k = j + 1; k < item_count; k++) {
                    int item1 = items[i];
                    int item2 = items[j];
                    int item3 = items[k];
                    // アイテムを昇順にソート
                    if (item1 > item2) { int temp = item1; item1 = item2; item2 = temp; }
                    if (item2 > item3) { int temp = item2; item2 = item3; item3 = temp; }
                    if (item1 > item2) { int temp = item1; item1 = item2; item2 = temp; }
                    
                    struct triple *p = searchTriple(item1, item2, item3);
                    if (p != NULL) {
                        p->count++;
                    }
                }
            }
        }
        
        transaction_count++;
    }
    fclose(fp);

    printf("Total transactions processed: %d\n", transaction_count);
    outputFrequentTriples(transaction_count);
    freeHashTab();

    return 0;
}