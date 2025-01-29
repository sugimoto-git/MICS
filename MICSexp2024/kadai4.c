#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_SIZE 200000   // ハッシュテーブルサイズ(大規模なら適宜変更)
#define MAX_ITEMS_IN_TRANSACTION 20000

static double MIN_SUPPORT_RATIO = 0.0;
static double MIN_CONFIDENCE = 0.0;
static long long TOTAL_TRANSACTIONS = 0;

// ==================================================
// パス1用 (単一アイテム) の構造とハッシュ
// ==================================================
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
        if (p->item == item) return p;
        p = p->next;
    }
    return NULL;
}
struct itemNode* createItemNode(int item) {
    struct itemNode *n = (struct itemNode*)malloc(sizeof(struct itemNode));
    if (!n) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    n->item = item;
    n->count = 1;
    n->next = NULL;
    return n;
}
void insertOrUpdateItem(int item) {
    struct itemNode *found = searchItem(item);
    if (found) {
        found->count++;
    } else {
        int h = hashItem(item);
        struct itemNode *n = createItemNode(item);
        n->next = itemHash[h];
        itemHash[h] = n;
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

// ==================================================
// パス2用 (2アイテムペア) の構造とハッシュ
// ==================================================
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
int hashPair(int a, int b) {
    // a <= b 前提
    long long key = (long long)a*31LL + (long long)b;
    long long idx = key % BUCKET_SIZE;
    if (idx<0) idx+=BUCKET_SIZE;
    return (int)idx;
}
struct pairNode* searchPair(int a, int b) {
    if (a>b) {int t=a; a=b; b=t;}
    int h = hashPair(a,b);
    struct pairNode *p = pairHash[h];
    while (p) {
        if (p->item1==a && p->item2==b) return p;
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
    // a<b
    if (a>b) {int t=a; a=b; b=t;}
    node->item1 = a;
    node->item2 = b;
    node->count = 0;
    node->next = NULL;
    return node;
}
void insertPairCandidate(int a, int b) {
    if (a>b) {int t=a; a=b; b=t;}
    struct pairNode *found = searchPair(a,b);
    if (!found) {
        int h = hashPair(a,b);
        struct pairNode *n = createPairNode(a,b);
        n->next = pairHash[h];
        pairHash[h] = n;
    }
}
void incrementPairCount(int a, int b) {
    struct pairNode *p = searchPair(a,b);
    if (p) {
        p->count++;
    }
}
void freePairHash() {
    for (int i=0; i<BUCKET_SIZE; i++) {
        struct pairNode *p = pairHash[i];
        while(p) {
            struct pairNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        pairHash[i] = NULL;
    }
}

// ==================================================
// パス3用 (3アイテム) の構造とハッシュ
// ==================================================
struct tripleNode {
    int item1;
    int item2;
    int item3;
    long long count;
    struct tripleNode *next;
};
static struct tripleNode *tripleHash[BUCKET_SIZE];

void initTripleHash() {
    for (int i=0; i<BUCKET_SIZE; i++) {
        tripleHash[i] = NULL;
    }
}
int hashTriple(int a, int b, int c) {
    // a<b<c
    if (a>b){int t=a; a=b; b=t;}
    if (b>c){int t=b; b=c; c=t;}
    if (a>b){int t=a; a=b; b=t;}
    long long key = (long long)a*31LL + (long long)b;
    key = key*31LL + (long long)c;
    long long idx = key % BUCKET_SIZE;
    if (idx<0) idx+=BUCKET_SIZE;
    return (int)idx;
}
struct tripleNode* searchTriple(int a, int b, int c) {
    if (a>b){int t=a;a=b;b=t;}
    if (b>c){int t=b;b=c;c=t;}
    if (a>b){int t=a;a=b;b=t;}
    int h = hashTriple(a,b,c);
    struct tripleNode *p = tripleHash[h];
    while(p) {
        if (p->item1==a && p->item2==b && p->item3==c) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}
struct tripleNode* createTripleNode(int a, int b, int c) {
    struct tripleNode *node=(struct tripleNode*)malloc(sizeof(struct tripleNode));
    if (!node) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    // a<b<c
    if (a>b){int t=a;a=b;b=t;}
    if (b>c){int t=b;b=c;c=t;}
    if (a>b){int t=a;a=b;b=t;}
    node->item1=a; 
    node->item2=b; 
    node->item3=c; 
    node->count=0;
    node->next=NULL;
    return node;
}
void insertTripleCandidate(int a, int b, int c) {
    // 既に存在しないかチェック
    if (!searchTriple(a,b,c)) {
        int h = hashTriple(a,b,c);
        struct tripleNode *n = createTripleNode(a,b,c);
        n->next = tripleHash[h];
        tripleHash[h] = n;
    }
}
void incrementTripleCount(int a,int b,int c) {
    struct tripleNode *p = searchTriple(a,b,c);
    if (p) {
        p->count++;
    }
}
void freeTripleHash() {
    for (int i=0; i<BUCKET_SIZE; i++) {
        struct tripleNode *p = tripleHash[i];
        while(p) {
            struct tripleNode *tmp=p;
            p=p->next;
            free(tmp);
        }
        tripleHash[i]=NULL;
    }
}

// ==================================================
// トランザクションファイルの行数(=total_transactions)を数える
// ==================================================
long long countTransactions(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        exit(1);
    }
    long long total=0;
    char line[1024*10];
    while(fgets(line,sizeof(line),fp)) {
        char *p=strtok(line," \t\r\n");
        if(!p) continue;
        int tlen=atoi(p);
        if(tlen==-1) break;
        total++;
    }
    fclose(fp);
    return total;
}

// --------------------------------------------------
// Pass1: L1.dat 出力
// --------------------------------------------------
long long pass1_generateL1(const char *transaction_file, const char *l1_file) {
    initItemHash();

    FILE *fp = fopen(transaction_file,"r");
    if(!fp){
        fprintf(stderr,"Error: cannot open %s\n", transaction_file);
        exit(1);
    }
    long long transCount=0;
    char line[1024*10];
    while(fgets(line,sizeof(line),fp)){
        char *ptr=strtok(line," \t\r\n");
        if(!ptr) continue;
        int tlen=atoi(ptr);
        if(tlen==-1) break;
        for(int i=0;i<tlen;i++){
            ptr=strtok(NULL," \t\r\n");
            if(!ptr) break;
            int it=atoi(ptr);
            insertOrUpdateItem(it);
        }
        transCount++;
    }
    fclose(fp);

    // L1.dat 書き出し
    FILE *fout = fopen(l1_file,"w");
    if(!fout){
        fprintf(stderr,"Error: cannot open %s for writing\n", l1_file);
        exit(1);
    }
    // support >= min_sup
    for(int i=0;i<BUCKET_SIZE;i++){
        struct itemNode *p=itemHash[i];
        while(p){
            double sup=(double)p->count/(double)transCount;
            if(sup >= MIN_SUPPORT_RATIO){
                fprintf(fout, "%d %lld %.6f\n", p->item, p->count, sup);
            }
            p=p->next;
        }
    }
    fclose(fout);

    return transCount;  
}

// --------------------------------------------------
// Pass2: L2.dat 出力
//   - L1.dat 読み込み → C2
//   - トランザクション再スキャン → 頻度カウント
// --------------------------------------------------
long long pass2_generateL2(const char *transaction_file, const char *l1_file, long long total_t) {
    initPairHash();

    // A) L1.dat の読み込み
    FILE *fp_l1 = fopen(l1_file,"r");
    if(!fp_l1){
        fprintf(stderr,"Error: cannot open %s\n", l1_file);
        exit(1);
    }
    int *l1_items = (int*)malloc(sizeof(int)*200000);
    if(!l1_items){
        fprintf(stderr,"Error: malloc failed for l1_items\n");
        exit(1);
    }
    int l1_count=0;
    while(!feof(fp_l1)){
        int it;
        long long c;
        double sup;
        int r=fscanf(fp_l1, "%d %lld %lf",&it,&c,&sup);
        if(r==3){
            l1_items[l1_count++]=it;
        } else {
            break;
        }
    }
    fclose(fp_l1);

    // B) C2 生成
    for(int i=0;i<l1_count;i++){
        for(int j=i+1;j<l1_count;j++){
            insertPairCandidate(l1_items[i], l1_items[j]);
        }
    }
    free(l1_items);

    // C) トランザクション再スキャン, ペア頻度カウント
    FILE *fp = fopen(transaction_file,"r");
    if(!fp){
        fprintf(stderr,"Error: cannot open %s\n", transaction_file);
        exit(1);
    }
    while(1){
        char line[1024*10];
        if(!fgets(line,sizeof(line),fp)) break;
        char *ptr=strtok(line," \t\r\n");
        if(!ptr) continue;
        int tlen=atoi(ptr);
        if(tlen==-1) break;
        static int items[MAX_ITEMS_IN_TRANSACTION];
        int ac=0;
        for(int i=0;i<tlen;i++){
            ptr=strtok(NULL," \t\r\n");
            if(!ptr) break;
            items[ac++]=atoi(ptr);
            if(ac>=MAX_ITEMS_IN_TRANSACTION) break;
        }
        // ペアを列挙
        for(int i=0;i<ac;i++){
            for(int j=i+1;j<ac;j++){
                incrementPairCount(items[i], items[j]);
            }
        }
    }
    fclose(fp);

    // D) L2.dat 出力
    FILE *fout = fopen("L2.dat","w");
    if(!fout){
        fprintf(stderr,"Error: cannot open L2.dat for writing\n");
        exit(1);
    }
    long long found_pairs=0;
    for(int i=0;i<BUCKET_SIZE;i++){
        struct pairNode *p=pairHash[i];
        while(p){
            double sup=(double)p->count/(double)total_t;
            if(sup >= MIN_SUPPORT_RATIO){
                fprintf(fout, "%d %d %lld %.6f\n", p->item1,p->item2,p->count,sup);
                found_pairs++;
            }
            p=p->next;
        }
    }
    fclose(fout);

    return found_pairs;
}

// --------------------------------------------------
// Pass3: L3.dat 出力
//   - L2.dat 読み込み → C3生成 (Apriori結合)
//   - トランザクション再スキャン → 頻度カウント
//   - L3.dat に出力
// --------------------------------------------------

// L2を読み込むための一時構造
struct pairInfo {
    int a;
    int b;
    long long count;
    double sup;
};

static struct pairCheck {
    int a;
    int b;
    struct pairCheck *next;
} *pairCheckHash[BUCKET_SIZE];

void initPairCheck() {
    for (int i=0;i<BUCKET_SIZE;i++){
        pairCheckHash[i]=NULL;
    }
}
int hashPairCheck(int a,int b){
    if(a>b){int t=a;a=b;b=t;}
    long long key=(long long)a*31LL + (long long)b;
    long long idx = key % BUCKET_SIZE;
    if(idx<0) idx+=BUCKET_SIZE;
    return (int)idx;
}
void insertPairCheck(int a,int b){
    if(a>b){int t=a;a=b;b=t;}
    int h=hashPairCheck(a,b);
    struct pairCheck *pc=(struct pairCheck*)malloc(sizeof(struct pairCheck));
    pc->a=a; 
    pc->b=b; 
    pc->next=pairCheckHash[h];
    pairCheckHash[h]=pc;
}
int isFrequentPairCheck(int a,int b){
    if(a>b){int t=a;a=b;b=t;}
    int h=hashPairCheck(a,b);
    struct pairCheck *p=pairCheckHash[h];
    while(p){
        if(p->a==a && p->b==b) return 1;
        p=p->next;
    }
    return 0;
}
void freePairCheckHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        struct pairCheck *p=pairCheckHash[i];
        while(p){
            struct pairCheck *tmp=p;
            p=p->next;
            free(tmp);
        }
        pairCheckHash[i]=NULL;
    }
}

long long pass3_generateL3(const char *transaction_file, long long total_t) {
    initTripleHash();

    // A) L2.dat 読み込み → pair配列 と pairCheckハッシュ
    initPairCheck();
    FILE *fp = fopen("L2.dat","r");
    if(!fp){
        fprintf(stderr,"Error: cannot open L2.dat\n");
        exit(1);
    }
    int capacity=100000;
    struct pairInfo *pairs=(struct pairInfo*)malloc(sizeof(struct pairInfo)*capacity);
    if(!pairs){
        fprintf(stderr,"Error: malloc failed for pairs\n");
        exit(1);
    }
    int pair_count=0;
    while(!feof(fp)){
        int a,b;
        long long c;
        double sup;
        int r=fscanf(fp,"%d %d %lld %lf",&a,&b,&c,&sup);
        if(r==4){
            // pairCheckに登録
            insertPairCheck(a,b);
            // 配列にも
            if(pair_count>=capacity){
                capacity*=2;
                struct pairInfo *tmp=(struct pairInfo*)realloc(pairs,sizeof(struct pairInfo)*capacity);
                if(!tmp){
                    fprintf(stderr,"Error: realloc failed\n");
                    exit(1);
                }
                pairs=tmp;
            }
            pairs[pair_count].a=a;
            pairs[pair_count].b=b;
            pairs[pair_count].count=c;
            pairs[pair_count].sup=sup;
            pair_count++;
        } else {
            break;
        }
    }
    fclose(fp);

    // B) C3生成
    //   (a1,a2) と (a1,a3) -> (a1,a2,a3)
    //   さらに (a2,a3) も L2 にあるか確認
    for(int i=0;i<pair_count;i++){
        int a1=pairs[i].a;
        int a2=pairs[i].b;
        for(int j=i+1;j<pair_count;j++){
            int b1=pairs[j].a;
            int b2=pairs[j].b;
            // 結合条件: a1==b1 => (a1,a2,b2)
            if(a1==b1){
                // subset check: (a2,b2) in L2 ?
                if(a2!=b2 && isFrequentPairCheck(a2,b2)){
                    insertTripleCandidate(a1,a2,b2);
                }
            }
        }
    }

    free(pairs);

    // C) トランザクション再スキャン → triple count
    FILE *fp2 = fopen(transaction_file,"r");
    if(!fp2){
        fprintf(stderr,"Error: cannot open %s\n", transaction_file);
        exit(1);
    }
    while(1){
        char line[1024*10];
        if(!fgets(line,sizeof(line),fp2)) break;
        char *ptr=strtok(line," \t\r\n");
        if(!ptr) continue;
        int tlen=atoi(ptr);
        if(tlen==-1) break;
        static int items[MAX_ITEMS_IN_TRANSACTION];
        int ac=0;
        for(int i=0;i<tlen;i++){
            ptr=strtok(NULL," \t\r\n");
            if(!ptr) break;
            items[ac++]=atoi(ptr);
            if(ac>=MAX_ITEMS_IN_TRANSACTION) break;
        }
        // 3アイテム全列挙
        for(int i=0;i<ac;i++){
            for(int j=i+1;j<ac;j++){
                for(int k=j+1;k<ac;k++){
                    incrementTripleCount(items[i],items[j],items[k]);
                }
            }
        }
    }
    fclose(fp2);

    // D) L3.dat 出力
    FILE *fout=fopen("L3.dat","w");
    if(!fout){
        fprintf(stderr,"Error: cannot open L3.dat for writing\n");
        exit(1);
    }
    long long found_triples=0;
    for(int i=0;i<BUCKET_SIZE;i++){
        struct tripleNode *p=tripleHash[i];
        while(p){
            double sup=(double)p->count/(double)total_t;
            if(sup >= MIN_SUPPORT_RATIO){
                fprintf(fout, "%d %d %d %lld %.6f\n",
                    p->item1,p->item2,p->item3, p->count, sup);
                found_triples++;
            }
            p=p->next;
        }
    }
    fclose(fout);

    freePairCheckHash();
    return found_triples;
}

// --------------------------------------------------
// 相関ルール抽出
//   - L1.dat, L2.dat, L3.dat を再度読み込み
//   - confidence >= minconf を満たすルールを出力
// --------------------------------------------------

// ハッシュ: L1
struct itemCountNode {
    int item;
    long long count;
    struct itemCountNode *next;
};
static struct itemCountNode *itemCountHash[BUCKET_SIZE];
void initItemCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        itemCountHash[i]=NULL;
    }
}
int hashItemCount(int item){
    long long key=(long long)item*31LL;
    long long idx=key % BUCKET_SIZE;
    if(idx<0)idx+=BUCKET_SIZE;
    return (int)idx;
}
void insertItemCount(int item, long long c){
    int h=hashItemCount(item);
    struct itemCountNode *n=(struct itemCountNode*)malloc(sizeof(struct itemCountNode));
    n->item=item; 
    n->count=c;
    n->next=itemCountHash[h];
    itemCountHash[h]=n;
}
long long getItemCount(int item){
    int h=hashItemCount(item);
    struct itemCountNode *p=itemCountHash[h];
    while(p){
        if(p->item==item) return p->count;
        p=p->next;
    }
    return -1;
}
void freeItemCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        struct itemCountNode *p=itemCountHash[i];
        while(p){
            struct itemCountNode *tmp=p;
            p=p->next;
            free(tmp);
        }
        itemCountHash[i]=NULL;
    }
}

// ハッシュ: L2
struct pairCountNode {
    int a;
    int b;
    long long count;
    struct pairCountNode *next;
};
static struct pairCountNode *pairCountHash[BUCKET_SIZE];
void initPairCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        pairCountHash[i]=NULL;
    }
}
int hashPairCount(int a,int b){
    if(a>b){int t=a;a=b;b=t;}
    long long key=(long long)a*31LL + (long long)b;
    long long idx=key%BUCKET_SIZE;
    if(idx<0)idx+=BUCKET_SIZE;
    return (int)idx;
}
void insertPairCountVal(int a,int b,long long c){
    if(a>b){int t=a;a=b;b=t;}
    int h=hashPairCount(a,b);
    struct pairCountNode *n=(struct pairCountNode*)malloc(sizeof(struct pairCountNode));
    n->a=a; n->b=b; n->count=c;
    n->next=pairCountHash[h];
    pairCountHash[h]=n;
}
long long getPairCountVal(int a,int b){
    if(a>b){int t=a;a=b;b=t;}
    int h=hashPairCount(a,b);
    struct pairCountNode *p=pairCountHash[h];
    while(p){
        if(p->a==a && p->b==b) return p->count;
        p=p->next;
    }
    return -1;
}
void freePairCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        struct pairCountNode *p=pairCountHash[i];
        while(p){
            struct pairCountNode *tmp=p;
            p=p->next;
            free(tmp);
        }
        pairCountHash[i]=NULL;
    }
}

// ハッシュ: L3
struct tripleCountNode {
    int a;
    int b;
    int c;
    long long count;
    struct tripleCountNode *next;
};
static struct tripleCountNode *tripleCountHash[BUCKET_SIZE];
void initTripleCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        tripleCountHash[i]=NULL;
    }
}
int hashTripleCount(int a,int b,int c){
    if(a>b){int t=a;a=b;b=t;}
    if(b>c){int t=b;b=c;c=t;}
    if(a>b){int t=a;a=b;b=t;}
    long long key=(long long)a*31LL+(long long)b;
    key=key*31LL+(long long)c;
    long long idx=key%BUCKET_SIZE;
    if(idx<0)idx+=BUCKET_SIZE;
    return (int)idx;
}
void insertTripleCountVal(int a,int b,int c,long long cnt){
    // a<b<c
    if(a>b){int t=a;a=b;b=t;}
    if(b>c){int t=b;b=c;c=t;}
    if(a>b){int t=a;a=b;b=t;}
    int h=hashTripleCount(a,b,c);
    struct tripleCountNode *n=(struct tripleCountNode*)malloc(sizeof(struct tripleCountNode));
    n->a=a; n->b=b; n->c=c; n->count=cnt;
    n->next=tripleCountHash[h];
    tripleCountHash[h]=n;
}
long long getTripleCountVal(int a,int b,int c){
    if(a>b){int t=a;a=b;b=t;}
    if(b>c){int t=b;b=c;c=t;}
    if(a>b){int t=a;a=b;b=t;}
    int h=hashTripleCount(a,b,c);
    struct tripleCountNode *p=tripleCountHash[h];
    while(p){
        if(p->a==a && p->b==b && p->c==c) return p->count;
        p=p->next;
    }
    return -1;
}
void freeTripleCountHash(){
    for(int i=0;i<BUCKET_SIZE;i++){
        struct tripleCountNode *p=tripleCountHash[i];
        while(p){
            struct tripleCountNode *tmp=p;
            p=p->next;
            free(tmp);
        }
        tripleCountHash[i]=NULL;
    }
}

// --------------------------------------------------
// L1.dat, L2.dat, L3.dat の読み込み
// --------------------------------------------------
void loadL1(const char *filename){
    initItemCountHash();
    FILE *fp=fopen(filename,"r");
    if(!fp){
        fprintf(stderr,"Error: cannot open %s\n",filename);
        exit(1);
    }
    while(!feof(fp)){
        int it;
        long long c;
        double sup;
        int r=fscanf(fp,"%d %lld %lf",&it,&c,&sup);
        if(r==3){
            insertItemCount(it,c);
        } else {
            break;
        }
    }
    fclose(fp);
}
void loadL2(const char *filename){
    initPairCountHash();
    FILE *fp=fopen(filename,"r");
    if(!fp){
        fprintf(stderr,"Error: cannot open %s\n", filename);
        exit(1);
    }
    while(!feof(fp)){
        int a,b;
        long long c;
        double sup;
        int r=fscanf(fp,"%d %d %lld %lf",&a,&b,&c,&sup);
        if(r==4){
            insertPairCountVal(a,b,c);
        } else {
            break;
        }
    }
    fclose(fp);
}
void loadL3(const char *filename){
    initTripleCountHash();
    FILE *fp=fopen(filename,"r");
    if(!fp){
        fprintf(stderr,"Error: cannot open %s\n",filename);
        exit(1);
    }
    while(!feof(fp)){
        int a,b,c;
        long long cnt;
        double sup;
        int r=fscanf(fp,"%d %d %d %lld %lf",&a,&b,&c,&cnt,&sup);
        if(r==5){
            insertTripleCountVal(a,b,c,cnt);
        } else {
            break;
        }
    }
    fclose(fp);
}

// --------------------------------------------------
// 相関ルール抽出
//   1) L1からのルール: {X} => {Y} ... (実質ペアのルール)
//   2) L2からのルール: {a} => {b}, {b} => {a}  (ペアを分割)
//   3) L3からのルール: 
//      - {a} => {b,c}, {b} => {a,c}, {c} => {a,b}
//      - {a,b} => {c}, {a,c} => {b}, {b,c} => {a}
// --------------------------------------------------

// 1要素->1要素 (実質ペア)
void rulesFromL2() {
    // pairCountHash 全走査
    for(int i=0;i<BUCKET_SIZE;i++){
        struct pairCountNode *p=pairCountHash[i];
        while(p){
            int a=p->a;
            int b=p->b;
            long long pair_cnt=p->count; 
            // {a} => {b}
            long long a_cnt = getItemCount(a);
            if(a_cnt>0){
                double conf=(double)pair_cnt/(double)a_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)pair_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d} => {%d}, support=%.4f, confidence=%.4f\n",a,b,sup,conf);
                }
            }
            // {b} => {a}
            long long b_cnt = getItemCount(b);
            if(b_cnt>0){
                double conf=(double)pair_cnt/(double)b_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)pair_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d} => {%d}, support=%.4f, confidence=%.4f\n",b,a,sup,conf);
                }
            }
            p=p->next;
        }
    }
}

// 1要素->2要素, 2要素->1要素
void rulesFromL3() {
    // tripleCountHash 全走査
    for(int i=0;i<BUCKET_SIZE;i++){
        struct tripleCountNode *p=tripleCountHash[i];
        while(p){
            int a=p->a;
            int b=p->b;
            int c=p->c;
            long long triple_cnt=p->count;

            // (1) {a} => {b,c}
            long long a_cnt=getItemCount(a);
            if(a_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)a_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d} => {%d, %d}, support=%.4f, confidence=%.4f\n",
                        a,b,c, sup, conf);
                }
            }
            // (2) {b} => {a,c}
            long long b_cnt=getItemCount(b);
            if(b_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)b_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d} => {%d, %d}, support=%.4f, confidence=%.4f\n",
                        b,a,c, sup, conf);
                }
            }
            // (3) {c} => {a,b}
            long long c_cnt=getItemCount(c);
            if(c_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)c_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d} => {%d, %d}, support=%.4f, confidence=%.4f\n",
                        c,a,b, sup, conf);
                }
            }
            // (4) {a,b} => {c}
            long long ab_cnt=getPairCountVal(a,b);
            if(ab_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)ab_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d, %d} => {%d}, support=%.4f, confidence=%.4f\n",
                        a,b,c, sup, conf);
                }
            }
            // (5) {a,c} => {b}
            long long ac_cnt=getPairCountVal(a,c);
            if(ac_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)ac_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d, %d} => {%d}, support=%.4f, confidence=%.4f\n",
                        a,c,b, sup, conf);
                }
            }
            // (6) {b,c} => {a}
            long long bc_cnt=getPairCountVal(b,c);
            if(bc_cnt>0 && triple_cnt>0){
                double conf=(double)triple_cnt/(double)bc_cnt;
                if(conf>=MIN_CONFIDENCE){
                    double sup=(double)triple_cnt/(double)TOTAL_TRANSACTIONS;
                    printf("{%d, %d} => {%d}, support=%.4f, confidence=%.4f\n",
                        b,c,a, sup, conf);
                }
            }

            p=p->next;
        }
    }
}

// --------------------------------------------------
// メイン
//   ./kadai1234 <transaction_file> <minsup> <minconf>
// --------------------------------------------------
int main(int argc,char **argv){
    if(argc!=4){
        fprintf(stderr,"Usage: %s <transaction_file> <minsup> <minconf>\n", argv[0]);
        return 1;
    }
    const char *transaction_file=argv[1];
    MIN_SUPPORT_RATIO = atof(argv[2]);
    MIN_CONFIDENCE = atof(argv[3]);

    // 1) トランザクション数を数える
    TOTAL_TRANSACTIONS = countTransactions(transaction_file);

    // 2) pass1 => L1.dat
    long long total_t = pass1_generateL1(transaction_file, "L1.dat");
    printf("=== Pass1 -> L1.dat ===\n");
    printf("Total transactions: %lld\n", total_t);

    // 3) pass2 => L2.dat
    long long l2_count = pass2_generateL2(transaction_file, "L1.dat", total_t);
    printf("=== Pass2 -> L2.dat ===\n");
    printf("Found %lld frequent pairs\n", l2_count);

    // 4) pass3 => L3.dat
    long long l3_count = pass3_generateL3(transaction_file, total_t);
    printf("=== Pass3 -> L3.dat ===\n");
    printf("Found %lld frequent triples\n", l3_count);

    // メモリ解放(パス1,2,3)
    freeItemHash();
    freePairHash();
    freeTripleHash();

    // 5) 相関ルール抽出
    //    - L1.dat, L2.dat, L3.dat を読み込み
    //    - confidence >= minconf のルールを出力
    loadL1("L1.dat");
    loadL2("L2.dat");
    loadL3("L3.dat");

    printf("\n=== Association Rules (confidence >= %.2f) ===\n", MIN_CONFIDENCE);

    // (a) L2から 1->1 ルール
    rulesFromL2();
    // (b) L3から 1->2, 2->1 ルール
    rulesFromL3();

    // 解放
    freeItemCountHash();
    freePairCountHash();
    freeTripleCountHash();

    printf("\nProgram finished (kadai1234: path1->path2->path3->association-rules)\n");
    return 0;
}
