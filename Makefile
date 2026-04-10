PROJ_NAME = ted
ALUNO     = muriloP

CC     = gcc
CFLAGS = -ggdb -O0 -std=c99 -fstack-protector-all \
         -Werror=implicit-function-declaration \
         -Wall -Wextra
LDFLAGS = -O0

# Diretórios
SRC  = src
INC  = include
TST  = test
UNI  = unity

# =============================================================================
# Objetos dos módulos
# =============================================================================
OBJS = $(SRC)/hash_extensivel.o \
       $(SRC)/quadra.o          \
       $(SRC)/pessoa.o          \
       $(SRC)/leitorGeo.o      \
       $(SRC)/leitorPm.o

# =============================================================================
# Flags de include compartilhadas
# =============================================================================
INC_FLAGS = -I $(INC) -I $(UNI)

# =============================================================================
# Target principal: gera o executável ted dentro de src/
# =============================================================================
$(PROJ_NAME): $(OBJS) $(SRC)/main.o
	$(CC) $(LDFLAGS) -o $(SRC)/$(PROJ_NAME) $(OBJS) $(SRC)/main.o

ted: $(PROJ_NAME)

# =============================================================================
# Regra genérica para compilar .c → .o dentro de src/
# =============================================================================
$(SRC)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $(INC_FLAGS) $< -o $@

# =============================================================================
# Dependências explícitas de cada módulo
# =============================================================================
$(SRC)/hash_extensivel.o: $(SRC)/hash_extensivel.c $(INC)/hash_extensivel.h

$(SRC)/quadra.o: $(SRC)/quadra.c $(INC)/quadra.h

$(SRC)/pessoa.o: $(SRC)/pessoa.c $(INC)/pessoa.h

$(SRC)/leitorGeo.o: $(SRC)/leitorGeo.c $(INC)/leitorGeo.h \
                     $(INC)/hash_extensivel.h $(INC)/quadra.h

$(SRC)/leitorPm.o: $(SRC)/leitorPm.c $(INC)/leitorPm.h \
                    $(INC)/hash_extensivel.h $(INC)/pessoa.h

$(SRC)/main.o: $(SRC)/main.c \
               $(INC)/hash_extensivel.h $(INC)/quadra.h $(INC)/pessoa.h \
               $(INC)/leitorGeo.h $(INC)/leitorPm.h

# =============================================================================
# Testes unitários — um target por módulo
# =============================================================================

# hash_extensivel
tst_hash: $(SRC)/hash_extensivel.o $(TST)/test_hash.c $(UNI)/unity.c
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(SRC)/hash_extensivel.o \
	    $(TST)/test_hash.c $(UNI)/unity.c \
	    -o $(TST)/run_hash
	$(TST)/run_hash

# quadra
tst_quadra: $(SRC)/quadra.o $(TST)/test_quadra.c $(UNI)/unity.c
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(SRC)/quadra.o \
	    $(TST)/test_quadra.c $(UNI)/unity.c \
	    -o $(TST)/run_quadra
	$(TST)/run_quadra

# pessoa
tst_pessoa: $(SRC)/pessoa.o $(TST)/test_pessoa.c $(UNI)/unity.c
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(SRC)/pessoa.o \
	    $(TST)/test_pessoa.c $(UNI)/unity.c \
	    -o $(TST)/run_pessoa
	$(TST)/run_pessoa

# leitor_geo
tst_leitor_geo: $(SRC)/hash_extensivel.o $(SRC)/quadra.o $(SRC)/leitorGeo.o \
                $(TST)/test_leitorGeo.c $(UNI)/unity.c
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(SRC)/hash_extensivel.o $(SRC)/quadra.o $(SRC)/leitorGeo.o \
	    $(TST)/test_leitorGeo.c $(UNI)/unity.c \
	    -o $(TST)/run_leitor_geo
	$(TST)/run_leitor_geo

tst_leitor_pm: $(SRC)/hash_extensivel.o $(SRC)/pessoa.o $(SRC)/leitorPm.o \
               $(TST)/test_leitorPm.c $(UNI)/unity.c
	$(CC) $(CFLAGS) $(INC_FLAGS) \
	    $(SRC)/hash_extensivel.o $(SRC)/pessoa.o $(SRC)/leitorPm.o \
	    $(TST)/test_leitorPm.c $(UNI)/unity.c \
	    -o $(TST)/run_leitor_pm
	$(TST)/run_leitor_pm

# =============================================================================
# tstall — compila e executa todos os testes unitários
# =============================================================================
tstall: tst_hash tst_quadra tst_pessoa tst_leitor_geo tst_leitor_pm

# =============================================================================
# Limpeza
# =============================================================================
clean:
	rm -f $(SRC)/*.o $(SRC)/$(PROJ_NAME)
	rm -f $(TST)/run_hash $(TST)/run_quadra $(TST)/run_pessoa \
	      $(TST)/run_leitor_geo $(TST)/run_leitor_pm
	rm -f *.hf *.hfc *.hfd
	rm -f $(TST)/*.hf $(TST)/*.hfc $(TST)/*.hfd
	rm -f temp_*.geo temp_*.pm

.PHONY: ted tstall tst_hash tst_quadra tst_pessoa \
        tst_leitor_geo tst_leitor_pm clean