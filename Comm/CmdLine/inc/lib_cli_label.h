//-------------------------------------------------------------------------------------------------
//
//  File : lib_cli_label.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
// Email: aroyer.qc@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

#if (DIGINI_USE_CMD_LINE == DEF_ENABLED)

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------

//#include "vt100_cfg.h"

//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#if (CLI_USE_AT_PREFIX_ON_COMMAND == DEF_ENABLED)
#define CLI_RESPONSE_FOR_LABEL,            "AT%s = %s%s\n>"
#else
#define CLI_RESPONSE_FOR_LABEL,            "%s = %s%s\n>"
#endif

// Console Generic Label
#define CLI_LABEL_LANGUAGE_DEF(X_CLI_LBL_CFG) \
X_CLI_LBL_CFG( LBL_CLEAR_SCREEN,            "\033[2J\033[H",                            nullptr                                     ) \
X_CLI_LBL_CFG( LBL_RESET_TERMINAL,          "\033c\n",                                  nullptr                                     ) \
X_CLI_LBL_CFG( LBL_ERROR,                   "ERROR, %s",                                "ERREUR, %s"                                ) \
X_CLI_LBL_CFG( LBL_CMD_LINE_PROC_STARTED,   "Command Line Process Started\n>",          "Procédé Ligne de Commande, Démarré\n>"     ) \
X_CLI_LBL_CFG( LBL_COMMAND_IS_INVALID,      "\rCommand is Invalid\n>",                  "\rCommande Invalide\n>"                    ) \
X_CLI_LBL_CFG( LBL_MALFORMED_PACKED,        "\rMalformed Command\n>",                   "\rCommande Mal Formé\n>"                   ) \
X_CLI_LBL_CFG( LBL_BUFFER_OVERFLOW,         "\rBuffer Overflow\n>",                     "\rDébordement du Tampon\n"                 ) \
X_CLI_LBL_CFG( LBL_CMD_TIME_OUT,            " ... Command Timeout\n>",                  " ... Expiration du Délai de Commande\n>"   ) \
X_CLI_LBL_CFG( LBL_OK,                      "OK",                                       nullptr                                     ) \
X_CLI_LBL_CFG( LBL_CLI_RESPONSE,            CLI_RESPONSE_FOR_LABEL,                     nullptr                                     ) \


// this will be extended message support CLI_USE_EXTENDED_ERROR
#define CLI_LABEL_EXT_LANGUAGE_DEF(X_CLI_LBL_CFG) \
X_CLI_LBL_CFG( LBL_DENIED,                  "Denied",                                   "Refuser"                                   ) \
X_CLI_LBL_CFG( LBL_NO_READ_SUPPORT,         "No Read Support",                          "Aucun Support de lecture"                  ) \
X_CLI_LBL_CFG( LBL_NO_WRITE_SUPPORT         "No Write Support",                         "Aucun Support d'écriture"                  ) \
X_CLI_LBL_CFG( LBL_INVALID_PARAMETER,       "Invalid Parameter",                        "Paramêtre non valide"                      ) \
X_CLI_LBL_CFG( LBL_PASSWORD_INVALID         "Password Invalid",                         "Mot de Passe invalide"                     ) \
X_CLI_LBL_CFG( LBL_MEM_ALLOC_ERROR,         "Memory allocation Error",                  "Erreur d'Allocation Mémoire"               ) \
X_CLI_LBL_CFG( LBL_PLAIN_CMD_ONLY,          "Plain Command Only",                       "Commande Simple Uniquement"                ) \






//-------------------------------------------------------------------------------------------------

#endif // (DIGINI_USE_CMD_LINE == DEF_ENABLED)

