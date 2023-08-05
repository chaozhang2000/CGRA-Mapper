OUT_DIR=$1
NAME=$2
cd $OUT_DIR
DOT_FILE=$(find -name "*.dot")
dot -Tpng $DOT_FILE -o $NAME.png
