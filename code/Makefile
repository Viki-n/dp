all: test_splay test_rb test_multisplay test_tango

test_splay:
	gcc -Wall -Wextra -O3 -D SPLAY -o test_splay_time run_test.c
	gcc -Wall -Wextra -O3 -D SPLAY -D TOUCH -o test_splay_touch run_test.c

test_rb:
	gcc -Wall -Wextra -O3 -D REDBLACK -o test_rb_time run_test.c
	gcc -Wall -Wextra -O3 -D REDBLACK -D TOUCH -o test_rb_touch run_test.c


test_multisplay:
	gcc -Wall -Wextra -O3 -D MULTISPLAY -o test_multisplay_time run_test.c
	gcc -Wall -Wextra -O3 -D MULTISPLAY -D TOUCH -o test_multisplay_touch run_test.c


test_tango:
	gcc -Wall -Wextra -O3 -D TANGO -o test_tango_time run_test.c
	gcc -Wall -Wextra -O3 -D TANGO -D TOUCH -o test_tango_touch run_test.c


%: %.c *.h common.c
	gcc -Wall -Wextra -o $@ $<

