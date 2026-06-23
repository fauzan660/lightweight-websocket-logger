#pragma once
#include "sqlite3.h"

sqlite3 *init_db();
int callback(void *data, int argc, char **argv, char **azColName);
