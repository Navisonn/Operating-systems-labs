CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I.
LDFLAGS = -lm
DLFLAGS = -ldl

IMPL1 = impl1/libimpl1.so
IMPL2 = impl2/libimpl2.so

STATIC_TEST = test_static/static_test
DYNAMIC_TEST = test_dynamic/dynamic_test

.PHONY: all clean

all: $(IMPL1) $(IMPL2) $(STATIC_TEST) $(DYNAMIC_TEST)

# ---------- Библиотеки ----------

$(IMPL1): impl1/libimpl1.c contract.h
	$(CC) $(CFLAGS) -shared impl1/libimpl1.c -o $(IMPL1) $(LDFLAGS)

$(IMPL2): impl2/libimpl2.c contract.h
	$(CC) $(CFLAGS) -shared impl2/libimpl2.c -o $(IMPL2) $(LDFLAGS)

# ---------- Тест №1 ----------

$(STATIC_TEST): test_static/static_test.c contract.h $(IMPL1)
	$(CC) $(CFLAGS) test_static/static_test.c -Limpl1 -limpl1 -o $(STATIC_TEST) $(LDFLAGS)

# ---------- Тест №2 ----------

$(DYNAMIC_TEST): test_dynamic/dynamic_test.c contract.h
	$(CC) $(CFLAGS) test_dynamic/dynamic_test.c -o $(DYNAMIC_TEST) $(DLFLAGS)

# ---------- Очистка ----------

clean:
	rm -f $(IMPL1) $(IMPL2) $(STATIC_TEST) $(DYNAMIC_TEST)
