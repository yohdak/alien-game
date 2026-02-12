# --- SETTINGAN COMPILER ---
CXX      := g++
# Tambahkan flag -MMD -MP buat auto-generate dependensi header
CXXFLAGS := -std=c++17 -Wall -Wno-missing-braces -I. -MMD -MP
LDFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Nama file .exe yang mau dibuat
TARGET   := megabonk

# --- AUTO DETECT FILES ---
# Cari semua file .cpp secara rekursif di folder-folder project
SRCS     := main.cpp Game.cpp \
            $(wildcard Player/*.cpp) \
            $(wildcard Enemies/*.cpp) \
			$(wildcard Map/*.cpp) \
            $(wildcard Managers/*.cpp) \
            $(wildcard Systems/*.cpp)\
			

# Daftar Object files (.o)
OBJS     := $(SRCS:.cpp=.o)
# Daftar Dependency files (.d) - Ini rahasia biar .h kebaca
DEPS     := $(SRCS:.cpp=.d)

# --- RULES ---

all: $(TARGET)

# Linker
$(TARGET): $(OBJS)
	@echo "🔗 Linking $(TARGET)..."
	@$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "✅ Build Success! Run with ./$(TARGET)"

# Compiler (Otomatis bikin .o dan .d)
%.o: %.cpp
	@echo "🔨 Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# SUNTIKKAN DEPENDENSI KE MAKEFILE
# Tanda '-' biar nggak error kalau file .d belum ada (pas pertama kali run)
-include $(DEPS)

# Bersih-bersih total
clean:
	@echo "🧹 Cleaning up..."
	@rm -f $(OBJS) $(TARGET) $(DEPS)
	@echo "✨ Cleaned!"

.PHONY: all clean