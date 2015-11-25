# コンパイラ
CC :=clang++
# インクルードファイル等
CFLAGS :=`pkg-config opencv --cflags` `pkg-config opencv --libs`
LDFLAGS :=-std=c++11 -stdlib=libc++
# ディレクトリ内の全てのC++ファイルをコンパイル
SOURCES :=$(wildcard *.cpp)
# C++ファイルの.cppをとったものを実行ファイルの名前とする
EXECUTABLE :=a.out

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCES)
	$(CC) $< $(LDFLAGS) $(CFLAGS) -o $@

clean:
	rm -rf $(EXECUTABLE)
