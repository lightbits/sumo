#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
    TokenString,
    TokenStruct,
    TokenAsterisk,
    TokenClass,
    TokenComma,
    TokenSemicolon,
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

bool string_match(char *a, int len_a,
                  char *b, int len_b)
{
    int i = 0;
    while (i < len_a)
    {
        if (i == len_b)   return false;
        if (a[i] != b[i]) return false;
        i++;
    }
    return i == len_b;
}

int length_of_null_terminated_string(char *s)
{
    int length = 0;
    while (*s)
    {
        length++;
        s++;
    }
    return length;
}

bool matches_identifier(Token *token, char *identifier)
{
    return string_match(token->text, token->length,
                        identifier, length_of_null_terminated_string(identifier));
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

        case ';':
        {
            result.type = TokenSemicolon;
            lexer->at++;
        } break;

        case ',':
        {
            result.type = TokenComma;
            lexer->at++;
        } break;

        case '(':
        {
            result.type = TokenOpenParen;
            lexer->at++;
        } break;

        case ')':
        {
            result.type = TokenCloseParen;
            lexer->at++;
        } break;

        case '{':
        {
            result.type = TokenOpenBrace;
            lexer->at++;
        } break;

        case '}':
        {
            result.type = TokenCloseBrace;
            lexer->at++;
        } break;

        case '*':
        {
            result.type = TokenAsterisk;
            lexer->at++;
        } break;

        default:
        {
            if (is_valid_identifier_char(lexer->at[0]) &&
                !is_numerical(lexer->at[0]))
            {
                char *begin = lexer->at;
                while (is_valid_identifier_char(lexer->at[0]))
                    lexer->at++;
                result.type = TokenIdentifier;
                result.text = begin;
                result.length = lexer->at - begin;
            }
            else
            {
                result.type = TokenUnknown;
                result.length = 1;
                result.text = lexer->at;
                lexer->at++;
            }

            if (result.type == TokenIdentifier)
            {
                if      (matches_identifier(&result, "struct")) result.type = TokenStruct;
                else if (matches_identifier(&result, "class"))  result.type = TokenClass;
            }
        } break;
    }
    return result;
}

struct StructMember
{
    char *name;
    int length;
    StructMember *next;
};

// struct SeenMetaType
// {
//     char *name;
//     int length;
//     SeenMetaType *next;
// };

// struct SeenMetaTypes
// {
//     SeenMetaType *first;
// };

// void add_to_seen_meta_types(SeenMetaTypes *seen_types, char *name, int length)
// {
//     if (!seen_types->first)
//     {
//         seen_types->first = new SeenMetaType();
//         seen_types->first->name = name;
//         seen_types->first->length = length;
//         seen_types->first->next = 0;
//     }
//     else
//     {
//         SeenMetaType *type = seen_types->first;
//         SeenMetaType *prev_type = 0;
//         while (type)
//         {
//             if (string_match(name, length, type->name, type->length))
//                 return;
//             prev_type = type;
//             type = type->next;
//         }
//         prev_type->next = new SeenMetaType();
//         prev_type->next->name = name;
//         prev_type->next->length = length;
//         prev_type->next->next = 0;
//     }
// }

void parse_struct(Lexer *lexer)
{
    #if 0
    Token struct_name = get_token(lexer);
    bool parsing = true;
    bool on_member = false;
    Token member_type = {};
    while (parsing)
    {
        Token token = get_token(lexer);
        switch (token.type)
        {
            case TokenIdentifier:
            {
                if (!on_member)
                {
                    on_member = true;
                    member_type = token.type;
                }
                else
                {

                }
            } break;

            case TokenEof:
            case TokenSemicolon:
            {
                parsing = false;
            } break;
        }
    }
    #else
    Token struct_name = get_token(lexer);
        Token token = get_token(lexer);
        printf("MetaMemberData member_of_%.*s[] = {\n",
               struct_name.length, struct_name.text);
        while (token.type != TokenEof &&
               token.type != TokenCloseBrace)
        {
            if (token.type == TokenIdentifier)
            {
                Token member_type = token;
                Token member_name = {};
                StructMember first_member = {};
                StructMember *current_member = &first_member;
                token = get_token(lexer);
                while (token.type != TokenEof &&
                       token.type != TokenSemicolon)
                {
                    if (token.type == TokenComma)
                    {
                        current_member->next = new StructMember();
                        current_member = current_member->next;
                    }
                    if (token.type == TokenIdentifier)
                    {
                        current_member->name = token.text;
                        current_member->length = token.length;
                    }
                    token = get_token(lexer);
                }

                StructMember *member = &first_member;
                while (member)
                {
                    printf("    { MetaType_%.*s, \"%.*s\", (u32)&(((%.*s*)0)->%.*s) },\n",
                            member_type.length, member_type.text,
                            member->length, member->name,
                            struct_name.length, struct_name.text,
                            member->length, member->name);
                    member = member->next;
                }
            }
            token = get_token(lexer);
        }
        printf("};\n");
    #endif
}

bool parse_introspectable(Lexer *lexer)
{
    Token introspectable = get_token(lexer);
    switch (introspectable.type)
    {
        case TokenStruct:
        {
            parse_struct(lexer);
            return true;
        } break;

        case TokenClass:
        {
            printf("Only introspection on structs is allowed\n");
            assert(false);
        } break;

        case TokenEof:
        {
            printf("Suddenly reached end of file\n");
            assert(false);
        } break;
    }
    return false;
}

int main(int argc, char **argv) // TODO: Take in file to introspect as argument
{
    if (argc < 2)
    {
        printf("No files passed in\n");
        return -1;
    }
    char *file = read_entire_file_and_null_terminate(argv[1]);
    if (!file)
    {
        printf("Wrong filename\n");
        return -1;
    }

    printf("// Generated code for introspection\n", argv[1]);

    // SeenMetaTypes seen_types = {};

    Lexer lexer = {};
    lexer.at = file;
    Token token = get_token(&lexer);
    while (token.type != TokenEof)
    {
        if (token.type == TokenIdentifier)
        {
            if (matches_identifier(&token, "__introspect"))
            {
                Lexer next = lexer;
                if (parse_introspectable(&next))
                    lexer = next;
            }
        }
        token = get_token(&lexer);
    }

    // printf("//enum MetaType\n//{\n");
    // SeenMetaType *type = seen_types.first;
    // while (type)
    // {
    //     printf("//    MetaType_%.*s,\n", type->length, type->name);
    //     type = type->next;
    // }
    // printf("//};\n");

    return 0;
}
