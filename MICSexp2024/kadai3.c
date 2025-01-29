#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_SIZE 200000   // ハッシュテーブルのサイズ(適宜調整)
#define MAX_ITEMS_IN_TRANSACTION 20000

// グローバルに最小支持度(割合)を保存
static double MIN_SUPPORT_RATIO = 0.0;

// ==================================================
// 1) パス1 (単一アイテム用)
// ==================================================
struct itemNode {
    int item;
    long long count;
    struct itemNode *next;
};

static struct itemNode *itemHash[BUCKET_SIZE];

// ハッシュ初期化
void initItemHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        itemHash[i] = NULL;
    }
}

int hashItem(int item) {
    long long key = (long long)item * 31LL;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE;
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

// ----------------------------
// Pass1: L1 出力
//   - transactionファイルを全件走査し、単一アイテムのcountを取得
//   - support >= MIN_SUPPORT_RATIO のものを L1.dat に書き出す
//   - 戻り値: 読み込んだトランザクション数
// ----------------------------
long long generateL1(const char *transaction_file, const char *l1_file) {
    initItemHash();

    FILE *fp = fopen(transaction_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s\n", transaction_file);
        exit(1);
    }

    long long total_transactions = 0;
    char line[1024 * 10];
    while (fgets(line, sizeof(line), fp)) {
        char *ptr = strtok(line, " \t\r\n");
        if (!ptr) continue;
        int tlen = atoi(ptr);
        if (tlen == -1) {
            break;
        }
        // トランザクション t のアイテム
        for (int i = 0; i < tlen; i++) {
            ptr = strtok(NULL, " \t\r\n");
            if (!ptr) break;
            int it = atoi(ptr);
            insertOrUpdateItem(it);
        }
        total_transactions++;
    }
    fclose(fp);

    // L1.dat に書き出し
    FILE *fout = fopen(l1_file, "w");
    if (!fout) {
        fprintf(stderr, "Error: cannot open %s for writing\n", l1_file);
        exit(1);
    }

    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct itemNode *p = itemHash[i];
        while (p) {
            double sup = (double)p->count / (double)total_transactions;
            if (sup >= MIN_SUPPORT_RATIO) {
                // item, count, support
                fprintf(fout, "%d %lld %.6f\n", p->item, p->count, sup);
            }
            p = p->next;
        }
    }
    fclose(fout);

    return total_transactions;
}

// ==================================================
// 2) パス2 (アイテムペア用)
// ==================================================
struct pairNode {
    int item1;
    int item2;
    long long count;
    struct pairNode *next;
};

static struct pairNode *pairHash[BUCKET_SIZE];

// ハッシュ初期化
void initPairHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        pairHash[i] = NULL;
    }
}

int hashPair(int a, int b) {
    // a <= b を前提
    long long key = (long long)a * 31LL + (long long)b;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE;
    return (int)idx;
}

struct pairNode* searchPair(int a, int b) {
    if (a > b) {
        int tmp = a; a = b; b = tmp;
    }
    int h = hashPair(a,b);
    struct pairNode *p = pairHash[h];
    while (p) {
        if (p->item1 == a && p->item2 == b) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

struct pairNode* createPairNode(int a, int b) {
    struct pairNode *node = (struct pairNode*)malloc(sizeof(struct pairNode));
    if (!node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    // a<bにそろえる
    if (a > b) { int tmp=a; a=b; b=tmp; }
    node->item1 = a;
    node->item2 = b;
    node->count = 0;
    node->next = NULL;
    return node;
}

void insertPairCandidate(int a, int b) {
    // a<bにそろえる
    if (a > b) {
        int tmp=a; a=b; b=tmp;
    }
    struct pairNode *found = searchPair(a, b);
    if (!found) {
        int h = hashPair(a,b);
        struct pairNode *n = createPairNode(a,b);
        n->next = pairHash[h];
        pairHash[h] = n;
    }
}

void incrementPairCount(int a, int b) {
    struct pairNode *p = searchPair(a, b);
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

// ----------------------------
// Pass2: (1) L1.dat を読み込み → C2生成
//        (2) 再スキャン → 頻度カウント
//        (3) L2.dat を出力
// ----------------------------
long long generateL2(const char *transaction_file, const char *l1_file, long long total_transactions) {
    initPairHash();

    // A) L1の読み込み
    FILE *fp_l1 = fopen(l1_file, "r");
    if (!fp_l1) {
        fprintf(stderr, "Error: cannot open %s\n", l1_file);
        exit(1);
    }
    int *l1_items = (int*)malloc(sizeof(int) * 200000);
    if (!l1_items) {
        fprintf(stderr, "Error: malloc failed for l1_items\n");
        exit(1);
    }
    int l1_count = 0;
    while (!feof(fp_l1)) {
        int it;
        long long cnt;
        double sup;
        int r = fscanf(fp_l1, "%d %lld %lf", &it, &cnt, &sup);
        if (r == 3) {
            l1_items[l1_count++] = it;
        } else {
            break;
        }
    }
    fclose(fp_l1);

    // B) C2 生成
    //    L1のアイテム全組合せ
    for (int i = 0; i < l1_count; i++) {
        for (int j = i+1; j < l1_count; j++) {
            insertPairCandidate(l1_items[i], l1_items[j]);
        }
    }
    free(l1_items);

    // C) C2 の頻度カウント(トランザクションを再スキャン)
    FILE *fp = fopen(transaction_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s\n", transaction_file);
        exit(1);
    }

    long long tcount = 0;
    char line[1024*10];
    while (fgets(line, sizeof(line), fp)) {
        char *ptr = strtok(line, " \t\r\n");
        if (!ptr) continue;
        int tlen = atoi(ptr);
        if (tlen == -1) {
            break;
        }
        static int items_in_t[MAX_ITEMS_IN_TRANSACTION];
        int ac = 0;
        for (int i = 0; i < tlen; i++) {
            ptr = strtok(NULL, " \t\r\n");
            if (!ptr) break;
            items_in_t[ac++] = atoi(ptr);
            if (ac >= MAX_ITEMS_IN_TRANSACTION) break;
        }
        // ペアを列挙してカウント
        for (int i = 0; i < ac; i++) {
            for (int j = i+1; j < ac; j++) {
                incrementPairCount(items_in_t[i], items_in_t[j]);
            }
        }
        tcount++;
    }
    fclose(fp);

    // D) L2.dat 出力
    FILE *fout = fopen("L2.dat", "w");
    if (!fout) {
        fprintf(stderr, "Error: cannot open L2.dat for writing\n");
        exit(1);
    }

    long long found_pairs = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct pairNode *p = pairHash[i];
        while (p) {
            double sup = (double)p->count / (double)total_transactions;
            if (sup >= MIN_SUPPORT_RATIO) {
                // item1 item2 count support
                fprintf(fout, "%d %d %lld %.6f\n", p->item1, p->item2, p->count, sup);
                found_pairs++;
            }
            p = p->next;
        }
    }
    fclose(fout);

    return found_pairs; // L2に含まれるペア数を返す
}

// ==================================================
// 3) パス3 (3アイテムセット)
// ==================================================
struct tripleNode {
    int item1;
    int item2;
    int item3;
    long long count;
    struct tripleNode *next;
};

static struct tripleNode *tripleHash[BUCKET_SIZE];

// 初期化
void initTripleHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        tripleHash[i] = NULL;
    }
}

// ハッシュ関数
int hashTriple(int a, int b, int c) {
    long long key = (long long)a * 31LL + (long long)b;
    key = key * 31LL + (long long)c;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE;
    return (int)idx;
}

// 検索
struct tripleNode* searchTriple(int a, int b, int c) {
    // a<b<c
    if (a > b) {int t=a; a=b; b=t;}
    if (b > c) {int t=b; b=c; c=t;}
    if (a > b) {int t=a; a=b; b=t;}

    int h = hashTriple(a,b,c);
    struct tripleNode *p = tripleHash[h];
    while (p) {
        if (p->item1 == a && p->item2 == b && p->item3 == c) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

// 作成
struct tripleNode* createTripleNode(int a, int b, int c) {
    struct tripleNode *node = (struct tripleNode*)malloc(sizeof(struct tripleNode));
    if (!node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    // a<b<c
    if (a > b) {int t=a; a=b; b=t;}
    if (b > c) {int t=b; b=c; c=t;}
    if (a > b) {int t=a; a=b; b=t;}

    node->item1 = a;
    node->item2 = b;
    node->item3 = c;
    node->count = 0;
    node->next = NULL;
    return node;
}

// 挿入(候補C3)
void insertTripleCandidate(int a, int b, int c) {
    if (!searchTriple(a,b,c)) {
        int h = hashTriple(a,b,c);
        struct tripleNode *node = createTripleNode(a,b,c);
        node->next = tripleHash[h];
        tripleHash[h] = node;
    }
}

// count++
void incrementTripleCount(int a, int b, int c) {
    struct tripleNode *p = searchTriple(a,b,c);
    if (p) {
        p->count++;
    }
}

void freeTripleHash() {
    for (int i = 0; i < BUCKET_SIZE; i++) {
        struct tripleNode *p = tripleHash[i];
        while (p) {
            struct tripleNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        tripleHash[i] = NULL;
    }
}

// ------------------------------------------
// L2を読み込み、pair配列に格納 & ハッシュ
// (item1 < item2) assumed
// ------------------------------------------
struct pairInfo {
    int item1;
    int item2;
    long long count;
    double support;
};

struct freqPairHash {
    int item1;
    int item2;
    struct freqPairHash *next;
};

static struct freqPairHash *pairCheck[BUCKET_SIZE];

void initPairCheck() {
    for (int i=0; i<BUCKET_SIZE; i++) {
        pairCheck[i] = NULL;
    }
}
int hashPairCheck(int a, int b) {
    long long key = (long long)a * 31LL + (long long)b;
    long long idx = key % BUCKET_SIZE;
    if (idx < 0) idx += BUCKET_SIZE;
    return (int)idx;
}
void insertFreqPair(int a, int b) {
    int h = hashPairCheck(a,b);
    struct freqPairHash *n = (struct freqPairHash*)malloc(sizeof(struct freqPairHash));
    n->item1 = a;
    n->item2 = b;
    n->next = pairCheck[h];
    pairCheck[h] = n;
}
int isFrequentPairCheck(int a, int b) {
    if (a>b) {int t=a; a=b; b=t;}
    int h = hashPairCheck(a,b);
    struct freqPairHash *p = pairCheck[h];
    while(p) {
        if (p->item1 == a && p->item2 == b) return 1;
        p = p->next;
    }
    return 0;
}
void freePairCheck() {
    for (int i=0; i<BUCKET_SIZE; i++) {
        struct freqPairHash *p = pairCheck[i];
        while(p) {
            struct freqPairHash *tmp = p;
            p = p->next;
            free(tmp);
        }
        pairCheck[i] = NULL;
    }
}

// ----------------------------
// Pass3: L2.dat → C3 → L3 出力
// ----------------------------
void generateL3(const char *transaction_file, long long total_transactions) {
    // 1) L2.dat を読み込む
    FILE *fp = fopen("L2.dat", "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open L2.dat\n");
        exit(1);
    }
    initPairCheck();

    int capacity = 100000;
    struct pairInfo *pairs = (struct pairInfo*)malloc(sizeof(struct pairInfo)*capacity);
    if (!pairs) {
        fprintf(stderr, "Error: malloc failed for pairs\n");
        exit(1);
    }
    int pair_count = 0;

    while (!feof(fp)) {
        int i1, i2;
        long long c;
        double sup;
        int r = fscanf(fp, "%d %d %lld %lf", &i1, &i2, &c, &sup);
        if (r == 4) {
            // 頻出ペア
            // ハッシュにも登録 (subset check用)
            insertFreqPair(i1, i2);

            // 配列にも格納
            if (pair_count >= capacity) {
                capacity *= 2;
                struct pairInfo *tmp = (struct pairInfo*)realloc(pairs, sizeof(struct pairInfo)*capacity);
                if (!tmp) {
                    fprintf(stderr, "Error: realloc failed\n");
                    exit(1);
                }
                pairs = tmp;
            }
            pairs[pair_count].item1 = i1;
            pairs[pair_count].item2 = i2;
            pairs[pair_count].count = c;
            pairs[pair_count].support = sup;
            pair_count++;
        } else {
            break;
        }
    }
    fclose(fp);

    // 2) C3生成 (Apriori結合)
    initTripleHash();
    for (int i = 0; i < pair_count; i++) {
        int a1 = pairs[i].item1;
        int a2 = pairs[i].item2;
        // => (a1 < a2)

        for (int j = i+1; j < pair_count; j++) {
            int b1 = pairs[j].item1;
            int b2 = pairs[j].item2;
            // => (b1 < b2)
            // 結合条件: a1 == b1 => (a1,a2,b2)
            if (a1 == b1) {
                // subset check: (a2,b2) も L2に含まれるか
                if (a2 != b2) {
                    if (isFrequentPairCheck(a2,b2)) {
                        // insert (a1,a2,b2)
                        insertTripleCandidate(a1,a2,b2);
                    }
                }
            }
        }
    }

    // 3) C3頻度カウント(再スキャン)
    FILE *fp2 = fopen(transaction_file, "r");
    if (!fp2) {
        fprintf(stderr, "Error: cannot open %s\n", transaction_file);
        exit(1);
    }
    char line[1024*10];
    long long tcount=0;
    while (fgets(line, sizeof(line), fp2)) {
        char *ptr = strtok(line, " \t\r\n");
        if (!ptr) continue;
        int tlen = atoi(ptr);
        if (tlen == -1) {
            break;
        }
        static int items[MAX_ITEMS_IN_TRANSACTION];
        int ac=0;
        for (int i=0; i<tlen; i++) {
            ptr = strtok(NULL, " \t\r\n");
            if (!ptr) break;
            items[ac++] = atoi(ptr);
            if (ac >= MAX_ITEMS_IN_TRANSACTION) break;
        }
        // 3アイテム列挙
        for (int i=0; i<ac; i++) {
            for (int j=i+1; j<ac; j++) {
                for (int k=j+1; k<ac; k++) {
                    incrementTripleCount(items[i], items[j], items[k]);
                }
            }
        }
        tcount++;
    }
    fclose(fp2);

    // 4) L3 出力
    printf("\n=== L3 (Frequent Triples) ===\n");
    long long printed=0;
    for (int i=0; i<BUCKET_SIZE; i++) {
        struct tripleNode *p = tripleHash[i];
        while (p) {
            double sup = (double)p->count / (double)total_transactions;
            if (sup >= MIN_SUPPORT_RATIO) {
                printf("(%d, %d, %d) count=%lld, support=%.4f\n",
                       p->item1, p->item2, p->item3, p->count, sup);
                printed++;
            }
            p = p->next;
        }
    }
    if (printed == 0) {
        printf("No triples meet the minimum support ratio (%.2f)\n", MIN_SUPPORT_RATIO);
    }

    // メモリ解放
    free(pairs);
    freePairCheck();
    freeTripleHash();
}


// ==================================================
// メイン: 
//   ./kadai123 <transaction_file> <min_support_ratio>
//
// フロー:
//   pass1 -> L1.dat
//   pass2 -> L2.dat
//   pass3 -> consoleにL3表示
// ==================================================
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <transaction_file> <min_support_ratio>\n", argv[0]);
        return 1;
    }

    const char *transaction_file = argv[1];
    MIN_SUPPORT_RATIO = atof(argv[2]);

    // 1) パス1 => L1.dat
    long long total_t = generateL1(transaction_file, "L1.dat");
    printf("=== L1 (single items) ===\n");
    printf("Output: L1.dat\n");
    printf("Total transactions: %lld\n", total_t);

    // ここで itemHash を解放
    freeItemHash();

    // 2) パス2 => L2.dat
    long long pair_count = generateL2(transaction_file, "L1.dat", total_t);
    printf("=== L2 (pairs) ===\n");
    printf("Output: L2.dat\n");
    printf("Found pairs: %lld\n", pair_count);

    // ここで pairHash を解放
    freePairHash();

    // 3) パス3 => コンソールに L3 表示
    //   - L2.dat から読み込み -> C3 -> 頻度カウント -> 出力
    generateL3(transaction_file, total_t);

    printf("\nProgram finished (path1->path2->path3).\n");
    return 0;
}
