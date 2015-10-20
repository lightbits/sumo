#include <stdio.h>
#include <stdlib.h>

char *read_entire_file_and_null_terminate(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return 0;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *result = new char[size + 1];
    if (!fread(result, 1, size, f))
        return 0;
    result[size] = '\0';
    fclose(f);
    return result;
}

enum TokenType
{
    TokenIdentifier,
    TokenStruct,
    TokenString,
    TokenOpenBrace,
    TokenCloseBrace,
    TokenOpenParen,
    TokenCloseParen,
    TokenEof,
    TokenUnknown
};

struct Token
{
    TokenType type;
    char *text;
    int length;
};

struct Lexer
{
    char *at;
};

bool is_newline(char c)
{
    return (c == '\n' ||
            c == '\r');
}

bool is_alphabetical(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}

bool is_numerical(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_valid_identifier_char(char c)
{
    return is_alphabetical(c) || is_numerical(c) || c == '_';
}

void eat_whitespace_and_comments(Lexer *lexer)
{
    while (lexer->at[0])
    {
        if (lexer->at[0] == ' ' || is_newline(lexer->at[0]))
        {
            lexer->at++;
        }
        else if (lexer->at[0] == '/' &&
                 lexer->at[1]        &&
                 lexer->at[1] == '/')
        {
            while (lexer->at[0])
            {
                if (is_newline(lexer->at[0]))
                    break;
                lexer->at++;
            }
        }
        else if (lexer->at[0] == '/' &&
                 lexer->at[1]        &&
                 lexer->at[1] == '*')
        {
            while (lexer->at[0])
            {
                if (lexer->at[1] && lexer->at[0] == '*' && lexer->at[1] == '/')
                {
                    lexer->at += 2;
                    break;
                }
                lexer->at++;
            }
        }
        else
        {
            break;
        }
    }
}

Token get_token(Lexer *lexer)
{
    Token result = {};
    eat_whitespace_and_comments(lexer);
    switch (lexer->at[0])
    {
        case '\0':
        {
            result.type = TokenEof;
            result.length = 1;
        } break;

        case '\"':
        {
            lexer->at++; // skip the first "
            char *begin = lexer->at;
            while (lexer->at[0] && lexer->at[0] != '\"')
                lexer->at++;

            lexer->at++; // skip the last "
            result.type = TokenString;
            result.text = begin;
            result.length = lexer->at - begin;
        } break;

        case '(':
        {
            result.type = TokenOpenParen;
            result.length = 1;
            lexer->at++;
        } break;

        case ')':
        {
            result.type = TokenCloseParen;
            result.length = 1;
            lexer->at++;
        } break;

        case '{':
        {
            result.type = TokenOpenBrace;
            result.length = 1;
            lexer->at++;
        } break;

        case '}':
        {
            result.type = TokenCloseBrace;
            result.length = 1;
            lexer->at++;
        } break;

        default:
        {
            if (is_alphabetical(lexer->at[0]))
            {
                char *begin = lexer->at;
                while (is_valid_identifier_char(lexer->at[0]))
                    lexer->at++;
                result.type = TokenIdentifier;
                result.text = begin;
                result.length = lexer->at - begin;
            }
            // else if (if_struct(lexer->at[0]))
            // {
            //     // TODO: Implement
            // }
            else
            {
                result.type = TokenUnknown;
                result.length = 1;
                lexer->at++;
            }
        } break;
    }
    return result;
}

int main(int argc, char **argv) // TODO: Take in file to introspect as argument
{
    char *file = read_entire_file_and_null_terminate("demo/introspection.cpp");
    if (!file)
    {
        printf("Wrong filename\n");
        return -1;
    }

    Lexer lexer = {};
    lexer.at = file;
    Token token = get_token(&lexer);
    while (token.type != TokenEof)
    {
        if (token.type == TokenIdentifier)
        {
            printf("%.*s\n", token.length, token.text);
        }
        token = get_token(&lexer);
    }

    return 0;
}
