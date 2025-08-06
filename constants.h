#ifndef _H_CONSTANTS
#define _H_CONSTANTS

// 基本サイズ定数
#define WORD_SIZE 8
#define MAX_FUNCTION_ARGS 6

// レジスタ定義
extern const char *ARG_REGISTERS_64[MAX_FUNCTION_ARGS];
extern const char *ARG_REGISTERS_32[MAX_FUNCTION_ARGS];
extern const char *ARG_REGISTERS_8[MAX_FUNCTION_ARGS];

// エラーメッセージ
#define ERR_MSG_UNDEFINED_VAR "変数が定義されていません"
#define ERR_MSG_NO_IDENTIFIER "識別子がありません"
#define ERR_MSG_NO_TYPE "型が存在しません"
#define ERR_MSG_LVAL_NOT_VAR "代入の左辺値が変数ではありません"
#define ERR_MSG_ARRAY_INDEX_NOT_NUM "グローバル変数の配列の添字は数値である必要があります"
#define ERR_MSG_FAILED_TO_ATTACH_MEMORIES "メモリ割り当てに失敗しました"
#define ERR_MSG_DETECT_UNPARSABLE_TYPE "対応していない型を検出しました"

#endif // _H_CONSTANTS
