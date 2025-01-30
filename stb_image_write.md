### Passos para baixar e instalar stb_image_write:
1. Abra o terminal.

2. Clone o repositório stb do GitHub:
´´´
git clone https://github.com/nothings/stb.git
´´´
3. Navegue até o diretório clonado:
´´´
cd stb
´´´
4. Copie o arquivo stb_image_write.h para o diretório do seu projeto:
´´´
cp stb_image_write.h /caminho/para/seu/projeto/
´´´
### Exemplo de uso no seu projeto:
1. Inclua o cabeçalho stb_image_write.h no seu código:
´´´
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
´´´
2. Compile seu código normalmente. Não é necessário nenhum passo adicional de instalação, pois stb_image_write é uma biblioteca de cabeçalho único.