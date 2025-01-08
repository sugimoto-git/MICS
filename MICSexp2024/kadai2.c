#include <stdio.h>
#include <stdlib.h>

#define BUCKET_SIZE 100  // ハッシュ表のサイズ
#define MIN_SUPPORT 2    // 最小支持度

// アイテムペアを保持する構造体の定義
struct pair {
    int item1;          // アイテム1
    int item2;          // アイテム2
    int count;          // 頻度
    struct pair *next;  // 次のペアへのポインタ
};

static struct pair *htab[BUCKET_SIZE];  // ハッシュ表: 各バケットの先頭のペアへのポインタ

// ハッシュ表の配列を初期化
void initHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        htab[i] = NULL;
    }
}

// ハッシュ値の計算
int hash(int key1, int key2) {
    return (key1 * 31 + key2) % BUCKET_SIZE;
}

// ハッシュ表の検索
struct pair *searchHashTab(int key1, int key2) {
    struct pair *p;
    for (p = htab[hash(key1, key2)]; p != NULL; p = p->next) {
        if (key1 == p->item1 && key2 == p->item2) {
            return p;
        }
    }
    return NULL;
}

// 新たなpair領域を確保
struct pair *newPair() {
    struct pair *p;
    if ((p = (struct pair *)malloc(sizeof(struct pair))) == NULL) {
        fprintf(stderr, "Error: malloc\n");
        exit(1);
    }
    p->item1 = -1;
    p->item2 = -1;
    p->count = 0;
    p->next = NULL;
    return p;
}

// ハッシュ表に挿入または頻度を増加
void insertOrUpdateHashTab(int key1, int key2) {
    struct pair *p = searchHashTab(key1, key2);
    if (p != NULL) {
        p->count++;
    } else {
        p = newPair();
        p->item1 = key1;
        p->item2 = key2;
        p->count = 1;
        int h = hash(key1, key2);
        p->next = htab[h];
        htab[h] = p;
    }
}

// ハッシュ表に保持されたアイテムペアを出力
void scanHashTab(int min_support) {
    printf("Frequent item pairs:\n");
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pair *p = htab[i];
        while (p != NULL) {
            if (p->count >= min_support) {
                printf("Item pair: (%d, %d), Count: %d\n", p->item1, p->item2, p->count);
            }
            p = p->next;
        }
    }
}

// ハッシュ表の領域を解放
void freeHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pair *p = htab[i];
        while (p != NULL) {
            struct pair *temp = p->next;
            free(p);
            p = temp;
        }
    }
}

// メイン関数
int main() {
    FILE *fp = fopen("sampleTran.dat", "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file\n");
        return 1;
    }

    initHashTab();

    int transaction_count = 0;
    int item;
    int transaction[100];
    int transaction_size = 0;

    while (fscanf(fp, "%d", &item) != EOF) {
        if (item == -1) {
            for (int i = 0; i < transaction_size; i++) {
                for (int j = i + 1; j < transaction_size; j++) {
                    insertOrUpdateHashTab(transaction[i], transaction[j]);
                }
            }
            transaction_count++;
            transaction_size = 0;
        } else {
            transaction[transaction_size++] = item;
        }
    }
    fclose(fp);

    printf("Total transactions: %d\n", transaction_count);
    scanHashTab(MIN_SUPPORT);
    freeHashTab();

    return 0;
}
