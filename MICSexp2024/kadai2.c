#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Adjust these defines as needed
//
#define BUCKET_SIZE 500000  // Hash size (larger for big data)
#define MAX_ITEMS_IN_TRANSACTION 20000

// Print progress every N transactions
#define PROGRESS_STEP 10000

static double MIN_SUPPORT_RATIO = 0.0;

// ====================================================
// Pass1: single items
// ====================================================
struct itemNode {
    int item;
    long long count;
    struct itemNode *next;
};

static struct itemNode *itemHash[BUCKET_SIZE];

void initItemHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        itemHash[i] = NULL;
    }
}

static int hashItem(int item) {
    // Use long long to reduce risk of overflow
    long long key = (long long)item * 31LL;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE; // mod in C can be negative
    return (int)idx;
}

struct itemNode* searchItem(int item) {
    int h = hashItem(item);
    struct itemNode *p = itemHash[h];
    while (p) {
        if (p->item == item) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

struct itemNode* createItemNode(int item) {
    struct itemNode *node = (struct itemNode*)malloc(sizeof(struct itemNode));
    if (!node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    node->item = item;
    node->count = 1;
    node->next = NULL;
    return node;
}

void insertOrUpdateItem(int item) {
    struct itemNode *found = searchItem(item);
    if (found) {
        found->count++;
    } else {
        int h = hashItem(item);
        struct itemNode *node = createItemNode(item);
        node->next = itemHash[h];
        itemHash[h] = node;
    }
}

void freeItemHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct itemNode *p = itemHash[i];
        while (p) {
            struct itemNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        itemHash[i] = NULL;
    }
}

// ====================================================
// Pass2: item pairs
// ====================================================
struct pairNode {
    int item1;
    int item2;
    long long count;
    struct pairNode *next;
};

static struct pairNode *pairHash[BUCKET_SIZE];

void initPairHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        pairHash[i] = NULL;
    }
}

static int hashPair(int item1, int item2) {
    // Ensure item1 <= item2
    if (item1 > item2) {
        int tmp = item1;
        item1 = item2;
        item2 = tmp;
    }
    long long key = (long long)item1 * 31LL + (long long)item2;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE;
    return (int)idx;
}

struct pairNode* searchPair(int item1, int item2) {
    if (item1 > item2) {
        int tmp = item1;
        item1 = item2;
        item2 = tmp;
    }
    int h = hashPair(item1, item2);
    struct pairNode *p = pairHash[h];
    while (p) {
        if (p->item1 == item1 && p->item2 == item2) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

struct pairNode* createPairNode(int item1, int item2) {
    struct pairNode *node = (struct pairNode*)malloc(sizeof(struct pairNode));
    if (!node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    // item1 <= item2 は外で処理済みと仮定
    node->item1 = item1;
    node->item2 = item2;
    node->count = 0;
    node->next = NULL;
    return node;
}

void insertPairCandidate(int item1, int item2) {
    // Ensure item1 <= item2
    if (item1 > item2) {
        int tmp = item1;
        item1 = item2;
        item2 = tmp;
    }
    struct pairNode *found = searchPair(item1, item2);
    if (!found) {
        int h = hashPair(item1, item2);
        struct pairNode *n = createPairNode(item1, item2);
        n->next = pairHash[h];
        pairHash[h] = n;
    }
}

void incrementPairCount(int item1, int item2) {
    if (item1 > item2) {
        int tmp = item1;
        item1 = item2;
        item2 = tmp;
    }
    struct pairNode *p = searchPair(item1, item2);
    if (p) {
        p->count++;
    }
}

void freePairHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pairNode *p = pairHash[i];
        while (p) {
            struct pairNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        pairHash[i] = NULL;
    }
}

// ====================================================
// Pass1: Generate L1
//   Return: total number of transactions
// ====================================================
long long generateL1(const char *transaction_file, const char *l1_file) {
    initItemHash();

    FILE *fp = fopen(transaction_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s\n", transaction_file);
        exit(1);
    }

    long long total_transactions = 0;
    char line[1024 * 10]; // 1行分バッファ(適宜拡張)

    while (fgets(line, sizeof(line), fp)) {
        char *ptr = strtok(line, " \t\r\n");
        if (!ptr) {
            continue; // 空行や読み取り不可
        }
        int tlen = atoi(ptr);
        if (tlen == -1) {
            // データ終端
            break;
        }

        // トランザクション内アイテムを読み取る
        for (int i = 0; i < tlen; i++) {
            ptr = strtok(NULL, " \t\r\n");
            if (!ptr) {
                // 行が途中で終わった場合
                break;
            }
            int it = atoi(ptr);
            insertOrUpdateItem(it);
        }

        total_transactions++;

        // Progress log for large data
        if (total_transactions % PROGRESS_STEP == 0) {
            fprintf(stderr, "[Pass1] Processed %lld transactions...\n", total_transactions);
        }
    }
    fclose(fp);

    // L1をファイルへ出力
    FILE *fout = fopen(l1_file, "w");
    if (!fout) {
        fprintf(stderr, "Error: cannot open %s for writing\n", l1_file);
        exit(1);
    }

    // 最小支持度判定
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct itemNode *p = itemHash[i];
        while (p) {
            double sup = (double)p->count / (double)total_transactions;
            if (sup >= MIN_SUPPORT_RATIO) {
                // item, count, support
                fprintf(fout, "%d %lld %.8f\n", p->item, p->count, sup);
            }
            p = p->next;
        }
    }
    fclose(fout);

    return total_transactions;
}

// ====================================================
// Pass2: Generate L2
// ====================================================
void generateL2(const char *transaction_file, const char *l1_file, long long total_transactions) {
    initPairHash();

    // L1を読み込む
    FILE *fpL1 = fopen(l1_file, "r");
    if (!fpL1) {
        fprintf(stderr, "Error: cannot open L1 file %s\n", l1_file);
        exit(1);
    }

    // L1アイテムを配列に格納
    int capacity = 100000; // L1に含まれるアイテム数の最大想定
    int *l1_items = (int*)malloc(sizeof(int) * capacity);
    if (!l1_items) {
        fprintf(stderr, "Error: malloc failed for l1_items\n");
        exit(1);
    }

    int l1_count = 0;
    while (!feof(fpL1)) {
        int it;
        long long c;
        double s;
        int r = fscanf(fpL1, "%d %lld %lf", &it, &c, &s);
        if (r == 3) {
            if (l1_count >= capacity) {
                // Realloc if needed
                capacity *= 2;
                int *tmp = (int*)realloc(l1_items, sizeof(int)*capacity);
                if (!tmp) {
                    fprintf(stderr, "Error: realloc failed\n");
                    exit(1);
                }
                l1_items = tmp;
            }
            l1_items[l1_count++] = it;
        } else {
            break;
        }
    }
    fclose(fpL1);

    // C2生成 (L1のアイテムペア)
    for (int i = 0; i < l1_count; i++) {
        for (int j = i + 1; j < l1_count; j++) {
            insertPairCandidate(l1_items[i], l1_items[j]);
        }
    }
    free(l1_items);

    // もう一度トランザクションファイルを読み込んでcount
    FILE *fp = fopen(transaction_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s\n", transaction_file);
        exit(1);
    }

    long long trans_count = 0;
    char line[1024 * 10];
    while (fgets(line, sizeof(line), fp)) {
        char *ptr = strtok(line, " \t\r\n");
        if (!ptr) {
            continue;
        }
        int tlen = atoi(ptr);
        if (tlen == -1) {
            break;
        }

        static int transItems[MAX_ITEMS_IN_TRANSACTION];
        int actualCount = 0;
        for (int i = 0; i < tlen; i++) {
            ptr = strtok(NULL, " \t\r\n");
            if (!ptr) break;
            transItems[actualCount++] = atoi(ptr);
            if (actualCount >= MAX_ITEMS_IN_TRANSACTION) {
                // 大きすぎるトランザクションはここで打ち切る
                break;
            }
        }
        // ペアのcount++
        for (int i = 0; i < actualCount; i++) {
            for (int j = i + 1; j < actualCount; j++) {
                incrementPairCount(transItems[i], transItems[j]);
            }
        }

        trans_count++;
        // 進捗表示
        if (trans_count % PROGRESS_STEP == 0) {
            fprintf(stderr, "[Pass2] Processed %lld transactions...\n", trans_count);
        }
    }
    fclose(fp);

    // L2をコンソールへ出力
    printf("\n=== L2 (Frequent Pairs) ===\n");
    long long printed_count = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pairNode *p = pairHash[i];
        while (p) {
            double sup = (double)p->count / (double)total_transactions;
            if (sup >= MIN_SUPPORT_RATIO) {
                printf("(%d, %d) count=%lld, support=%.4f\n",
                       p->item1, p->item2, p->count, sup);
                printed_count++;
            }
            p = p->next;
        }
    }

    if (printed_count == 0) {
        printf("No pairs meet the minimum support ratio (%.2f).\n", MIN_SUPPORT_RATIO);
    }
}

// ====================================================
// main
// ====================================================
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <transaction_file> <min_support_ratio>\n", argv[0]);
        return 1;
    }

    const char *transaction_file = argv[1];
    MIN_SUPPORT_RATIO = atof(argv[2]);

    // Pass1
    long long total_t = generateL1(transaction_file, "L1.dat");

    printf("=== L1 (Frequent single items) ===\n");
    printf("Output file: L1.dat\n");
    printf("Total transactions: %lld\n", total_t);

    freeItemHash();

    // Pass2
    generateL2(transaction_file, "L1.dat", total_t);

    freePairHash();

    printf("\nProgram finished successfully.\n");
    return 0;
}
