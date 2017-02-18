#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>

#include <json.h>

int
checkmail(DIR *dir)
{
	int res = 0;

	rewinddir(dir);

	readdir(dir);
	readdir(dir);

	while (readdir(dir))
		res++;

	return res;
}

int
main(int argc, char **argv)
{
	DIR *dir;
	char *buf = NULL;
	char *mailbox = NULL;
	char result[32];
	const char *errstr;
	size_t len;
	int newmail, i, ch;
	int position = -1;
	int comma = 0;
	struct json_object *json;
	struct json_object *mail;
	struct json_object *array;
	struct json_object *nomail;

	if (pledge("stdio rpath", NULL) == -1)
		errx(1, "pledge");

	while ((ch = getopt(argc, argv, "m:p:")) != -1) {
		switch (ch) {
		case 'm':
			mailbox = optarg;
			break;
		case 'p':
			position = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr)
				errx(1, "%s is %s", optarg, errstr);
			break;
		default:
			exit(1);
			break;
		}
	}

	if (!mailbox)
		errx(1, "mailbox must be specified (-m)");

	if (position == -1)
		errx(1, "position must be specified (-p)");

	/* skip the first two lines which are procedural output */
	for (i = 0; i < 2; i++) {
		if (getline(&buf, &len, stdin) == -1)
			errx(1, "getline");
		printf("%s", buf);
	}

	if ((dir = opendir(mailbox)) == NULL)
		errx(1, "opendir");

	nomail = json_object_new_object();
	json_object_object_add(nomail, "name", json_object_new_string("mail"));
	json_object_object_add(nomail, "instance", json_object_new_string(mailbox));
	json_object_object_add(nomail, "full_text", json_object_new_string(""));
	json_object_object_add(nomail, "separator", json_object_new_boolean(FALSE));
	json_object_object_add(nomail, "color", json_object_new_string("#FFFF00"));


	while (getline(&buf, &len, stdin) != -1) {
		comma = 0;
		if (strncmp(buf, ",", 1) == 0) {
			comma = 1;
		}

		if ((json = json_tokener_parse(buf + comma)) == NULL)
			errx(1, "json parsing failed");

		if ((newmail = checkmail(dir)) == 0) {
			mail = json_object_get(nomail);
		} else {
			snprintf(result, sizeof(result), " %i ", newmail);
			mail = json_object_new_object();
			json_object_object_add(mail, "name", json_object_new_string("mail"));
			json_object_object_add(mail, "instance", json_object_new_string(mailbox));
			json_object_object_add(mail, "full_text", json_object_new_string(result));
			json_object_object_add(mail, "separator", json_object_new_boolean(FALSE));
			json_object_object_add(mail, "color", json_object_new_string("#FFFF00"));
		}

		array = json_object_new_array();

		for (i = 0; i < json_object_array_length(json); i++) {
			if (i == position)
				json_object_array_add(array, mail);
			json_object_array_add(array, json_object_get(json_object_array_get_idx(json, i)));
		}

		if (comma)
			printf(",");
		printf("%s\n", json_object_to_json_string_ext(array, JSON_C_TO_STRING_PLAIN));
		fflush(stdout);

		json_object_put(json);
		json_object_put(array);
	}

	return 1; /* only reached in error */
}
