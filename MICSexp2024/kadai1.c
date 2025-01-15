#include <stdio.h>
#include <stdlib.h>

#define BUCKET_SIZE 1000  // ハッシュ表のサイズ
#define MIN_SUPPORT 50     // 最小支持度を低く設定

// アイテムを保持する構造体の定義
struct cell {
    int item;           // アイテムを保持
    int count;          // 頻度を保持
    struct cell *next;  // 次のセルへのポインタ
};

static struct cell *htab[BUCKET_SIZE];  // ハッシュ表: 各バケットの先頭のセルへのポインタ

// ハッシュ表の配列を初期化
void initHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        htab[i] = NULL;
    }
}

// ハッシュ値の計算
int hash(int key) {
    return key % BUCKET_SIZE;
}

// ハッシュ表の検索
struct cell *searchHashTab(int key) {
    struct cell *p;
    for (p = htab[hash(key)]; p != NULL; p = p->next) {
        if (key == p->item) {
            return p;
        }
    }
    return NULL;
}

// 新たなcell領域を確保
struct cell *newCell() {
    struct cell *p;
    if ((p = (struct cell *)malloc(sizeof(struct cell))) == NULL) {
        fprintf(stderr, "Error: malloc\n");
        exit(1);
    }
    p->item = -1;
    p->count = 0;
    p->next = NULL;
    return p;
}

// ハッシュ表に挿入または頻度を増加
void insertOrUpdateHashTab(int key) {
    struct cell *p = searchHashTab(key);
    if (p != NULL) {
        p->count++;
    } else {
        p = newCell();
        p->item = key;
        p->count = 1;
        int h = hash(key);
        p->next = htab[h];
        htab[h] = p;
    }
}

// ハッシュ表に保持されたアイテムを出力
void scanHashTab(int min_support) {
    printf("Frequent items:\n");
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct cell *p = htab[i];
        while (p != NULL) {
            if (p->count >= min_support) {
                printf("Item: %d, Count: %d\n", p->item, p->count);
            }
            p = p->next;
        }
    }
}

// ハッシュ表の領域を解放
void freeHashTab() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct cell *p = htab[i];
        while (p != NULL) {
            struct cell *temp = p->next;
            free(p);
            p = temp;
        }
    }
}

// メイン関数
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <transaction file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", argv[1]);
        return 1;
    }

    initHashTab();

    int transaction_count = 0;
    int tlen, item;
    while (fscanf(fp, "%d", &tlen) != EOF) {
        if (tlen == -1) break;
        for (int i = 0; i < tlen; i++) {
            fscanf(fp, "%d", &item);
            insertOrUpdateHashTab(item);
        }
        transaction_count++;
    }
    fclose(fp);

    printf("Total transactions: %d\n", transaction_count);
    scanHashTab(MIN_SUPPORT);
    freeHashTab();

    return 0;
}