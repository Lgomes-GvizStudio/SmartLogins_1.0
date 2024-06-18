#include "sqlite/sqlite_db.h"
#include "include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_CHARS 50
#define MAX_ENTRIES 10

typedef struct {
    int id;
    char word1[MAX_INPUT_CHARS + 1];
    char word2[MAX_INPUT_CHARS + 1];
} WordPair;

// Definição da estrutura Button para criar botões na interface
typedef struct Button {
    Rectangle bounds; // Área do botão
    const char *text; // Texto do botão
    Color color; // Cor padrão do botão
    Color hoverColor; // Cor do botão quando o mouse está sobre ele
    Color pressedColor; // Cor do botão quando está sendo pressionado
    Texture2D texture;
    Texture2D hoverTexture;
    Texture2D pressedTexture;
} Button;
// Função para desenhar um botão na tela
void DrawButton(Button button) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePoint, button.bounds);
    bool isPressed = isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    if (isPressed) {
        DrawRectangleRec(button.bounds, button.pressedColor);
        DrawTexture(button.pressedTexture, button.bounds.x, button.bounds.y, WHITE);
    } else if (isHovered) {
        DrawRectangleRec(button.bounds, button.hoverColor);
        DrawTexture(button.hoverTexture, button.bounds.x, button.bounds.y, WHITE);
    } else {
        DrawRectangleRec(button.bounds, button.color);
        DrawTexture(button.texture, button.bounds.x, button.bounds.y, WHITE);
    }

    int textWidth = MeasureText(button.text, 20);
    int textHeight = 20; // Assumindo tamanho de fonte 20
    DrawText(button.text,
             button.bounds.x + (button.bounds.width - textWidth) / 2,
             button.bounds.y + (button.bounds.height - textHeight) / 2,
             20,
             BLACK);
}

// Função para verificar se um botão foi clicado
bool IsButtonClicked(Button button) {
    Vector2 mousePoint = GetMousePosition();
    return CheckCollisionPointRec(mousePoint, button.bounds) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

int index = 0; // Global variable to use as an index for fetching data

// Função para inicializar o banco de dados
void InitDatabase(sqlite3 **db) {
    int rc = sqlite3_open("words.db", db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
        exit(1);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    char *sql = "CREATE TABLE IF NOT EXISTS WORD_PAIRS("
                "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                "WORD1 TEXT NOT NULL,"
                "WORD2 TEXT NOT NULL);";
    char *zErrMsg = 0;
    rc = sqlite3_exec(*db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        exit(1);
    } else {
        fprintf(stderr, "Table created successfully\n");
    }
}

// Função para adicionar um par de palavras ao banco de dados
void AddWordPair(sqlite3 *db, const char *word1, const char *word2, int *entryCount) {
    char sql[512];
    snprintf(sql, sizeof(sql), "INSERT INTO WORD_PAIRS (WORD1, WORD2) VALUES ('%s', '%s');", word1, word2);
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stderr, "Word pair added successfully\n");
        (*entryCount)++;
    }
}

// Função para buscar todos os pares de palavras do banco de dados
int FetchWordPairs(void *data, int argc, char **argv, char **azColName) {
    WordPair *wordPairs = (WordPair *)data;

    wordPairs[index].id = atoi(argv[0]);
    strcpy(wordPairs[index].word1, argv[1]);
    strcpy(wordPairs[index].word2, argv[2]);
    index++;

    return 0;
}

void GetWordPairs(sqlite3 *db, WordPair *wordPairs, int *entryCount) {
    char *sql = "SELECT ID, WORD1, WORD2 FROM WORD_PAIRS;";
    char *zErrMsg = 0;
    index = 0; // Reset the index before fetching
    int rc = sqlite3_exec(db, sql, FetchWordPairs, wordPairs, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stderr, "GetWordPairs successfully\n");
    }
}

// Função para excluir um par de palavras do banco de dados
void DeleteWordPair(sqlite3 *db, int id, int *entryCount) {
    char sql[512];
    snprintf(sql, sizeof(sql), "DELETE FROM WORD_PAIRS WHERE ID=%d;", id);
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stderr, "Word pair deleted successfully\n");
        (*entryCount)--;
    }
}

int main(void) {
    // Inicializar o banco de dados
    sqlite3 *db;
    InitDatabase(&db);

    // Inicializar Raylib
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "SmartLogins v1.0");

    // Carrega um ícone para a janela
    Texture2D icon = LoadTexture("smartLogins_icon.png");
    Image Window_icon = LoadImageFromTexture(icon);
    SetWindowIcon(Window_icon);

    char input1[MAX_INPUT_CHARS + 1] = "\0";
    char input2[MAX_INPUT_CHARS + 1] = "\0";
    int letterCount1 = 0;
    int letterCount2 = 0;
    bool enterSecondWord = false;
    WordPair wordPairs[MAX_ENTRIES] = {0};
    int entryCount = 0;

    
    Texture2D buttonTexture = LoadTexture("info_icon.png");
    Texture2D hoverTexture = LoadTexture("info_icon_hover.png");
    Texture2D pressedTexture = LoadTexture("info_icon_click.png");

    Button myButton_img = {
        .bounds = { 5, 550, 100, 50 },
        .text = "",
        .texture = buttonTexture,
        .hoverTexture = hoverTexture,
        .pressedTexture = pressedTexture
    };

    // Carregar dados do banco de dados
    GetWordPairs(db, wordPairs, &entryCount);

    // Recuperar o número de entradas já existentes no banco de dados
    sqlite3_stmt *stmt;
    char *countQuery = "SELECT COUNT(*) FROM WORD_PAIRS;";
    if (sqlite3_prepare_v2(db, countQuery, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare count query\n");
        exit(1);
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entryCount = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Atualizar

                if (IsButtonClicked(myButton_img)) {
            // Define a ação ao clicar no botão "copy"
            OpenURL("https://www.instagram.com/gviz_studio/");  
        }

        if (entryCount < MAX_ENTRIES) {
            if (!enterSecondWord) {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (letterCount1 < MAX_INPUT_CHARS)) {
                        input1[letterCount1] = (char)key;
                        input1[letterCount1 + 1] = '\0';
                        letterCount1++;
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE) && letterCount1 > 0) {
                    letterCount1--;
                    if (letterCount1 < 0) letterCount1 = 0;
                    input1[letterCount1] = '\0';
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mousePoint = GetMousePosition();
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){650, 40, 100, 30})) {
                        enterSecondWord = true;
                    }
                }
            } else {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (letterCount2 < MAX_INPUT_CHARS)) {
                        input2[letterCount2] = (char)key;
                        input2[letterCount2 + 1] = '\0';
                        letterCount2++;
                    }
                    key = GetCharPressed();
                }

                if (IsKeyPressed(KEY_BACKSPACE) && letterCount2 > 0) {
                    letterCount2--;
                    if (letterCount2 < 0) letterCount2 = 0;
                    input2[letterCount2] = '\0';
                }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    Vector2 mousePoint = GetMousePosition();
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){650, 100, 100, 30})) {
                        if (letterCount1 > 0 && letterCount2 > 0) {
                            AddWordPair(db, input1, input2, &entryCount);
                            letterCount1 = 0;
                            letterCount2 = 0;
                            input1[0] = '\0';
                            input2[0] = '\0';
                            memset(wordPairs, 0, sizeof(wordPairs));
                            GetWordPairs(db, wordPairs, &entryCount);
                            enterSecondWord = false;
                        }
                    }
                }
            }
        }

        // Desenhar
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Seção de adicionar
        if (entryCount < MAX_ENTRIES) {
            if (!enterSecondWord) {
                DrawText("Login:", 10, 10, 20, DARKGRAY);
                DrawText(input1, 10, 40, 20, LIGHTGRAY);
                DrawRectangle(650, 40, 110, 30, LIGHTGRAY);
                DrawText("Adicionar", 660, 45, 20, DARKGRAY);
            } else {
                DrawText("Senha:", 10, 70, 20, DARKGRAY);
                DrawText(input2, 10, 100, 20, LIGHTGRAY);
                DrawRectangle(650, 100, 110, 30, LIGHTGRAY);
                DrawText("Adicionar", 660, 105, 20, DARKGRAY);
            }
        } else {
            DrawText("Maximo de logins adicionados", 10, 10, 20, DARKGRAY);
        }

        // Seção de exibir dados já adicionados
        DrawText("Login's e senhas :", 10, 140, 20, DARKGRAY);
        int y = 170;
        for (int i = 0; i < entryCount; i++) {
            // Exibir o primeiro dado
            DrawText(wordPairs[i].word1, 30, y, 20, BLACK);

            // Exibir o segundo dado
            DrawText(wordPairs[i].word2, 280, y, 20, BLACK);

            // Desenhar o botão de exclusão
            DrawRectangle(520, y - 5, 100, 30, RED);
            DrawText("Deletar", 535, y, 20, DARKGRAY);

            // Desenhar o botão de cópia
            DrawRectangle(640, y - 5, 100, 30, BLUE);
            DrawText("Copiar", 660, y, 20, DARKGRAY);

            y += 40;
        }

         DrawButton(myButton_img);

        // Atualização ao clicar nos botões de exclusão e cópia
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePoint = GetMousePosition();
            y = 170;
            for (int i = 0; i < entryCount; i++) {
                if (CheckCollisionPointRec(mousePoint, (Rectangle){520, y - 5, 100, 30})) {
                    DeleteWordPair(db, wordPairs[i].id, &entryCount);
                    memset(wordPairs, 0, sizeof(wordPairs));
                    GetWordPairs(db, wordPairs, &entryCount);
                    break;
                } else if (CheckCollisionPointRec(mousePoint, (Rectangle){640, y - 5, 100, 30})) {
                    char clipboardText[MAX_INPUT_CHARS * 2 + 20];
                    snprintf(clipboardText, sizeof(clipboardText), "Login > %s / Senha >%s", wordPairs[i].word1, wordPairs[i].word2);
                    SetClipboardText(clipboardText);
                    break;
                }
                y += 40;
            }
        }

        // Draw entry count and max entries
        char entryCountText[32];
        sprintf(entryCountText, " %d/%d", entryCount, MAX_ENTRIES);
        DrawText(entryCountText, screenWidth - MeasureText(entryCountText, 20) - 10, screenHeight - 40, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    sqlite3_close(db);

    return 0;
}
