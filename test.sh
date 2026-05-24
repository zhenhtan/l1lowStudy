for file in *.txt; do
    echo "当前文件: $file"
    cat "$file"
    echo "------"
done
