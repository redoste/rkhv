#!/bin/sh
commit_title="$(head "$1" -n 1)"
if [ "$(echo "$commit_title" | tr -d '\n' | wc -c)" -gt 72 ]; then
	echo "Commit title is more than 72 characters long" >&2
	exit 1
fi

regex='^[a-z/{},]+ : [A-Z0-9].+$'
if ! echo "$commit_title" | grep -E "$regex" >/dev/null; then
	echo "Commit title doesn't match the \"subdir/category : Description\" format (\`$regex\`)" >&2
	exit 1
fi

exit 0
