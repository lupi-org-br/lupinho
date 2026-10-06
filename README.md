# 🎮 Lupinho
### Criado por: [Juneira](https://github.com/juneira)
**Lupinho** é um Simulador para o Console Brasileiro **Lupi**.  
Funciona diretamente no seu navegador usando WebAssembly, permitindo que crie e jogue jogos usando API de programação Lua.

### 🕹️ [Teste a Demo](https://lupinho.juneira.com/)

> O jogo de demonstração é ["caio-pernocas"](https://github.com/lupi-org-br/caio-pernocas).  
> Use as teclas W, A, S, D para mover e K como botão de ação. Você também pode usar um Controle.
### Simulador Vs Emulador 
É importante entender a diferença entre estes dois termos para saber exatamente como o Lupinho funciona:
### **Emulador**:  
  > Tenta recriar o hardware original de forma fiel e em nível de baixo nível (processador, memória, chips gráficos, etc.).  
  O objetivo é rodar o software original feito para aquela máquina física exata, processando instrução por instrução da CPU original.
### **Simulador**:  
  > Recria o comportamento e o ambiente de execução de um sistema, muitas vezes em um nível mais abstrato ou moderno, sem a necessidade de replicar o hardware original fisicamente.  
   No caso do Lupinho, ele simula o console Lupi diretamente sobre tecnologias modernas (como WebAssembly e Raylib), focando em rodar jogos criados especificamente para essa API, e não em rodar ROMs de um hardware físico pré-existente.  
    
Você pode ter mais informações sobre os **Limites** nesta Issue: [**LIMITAÇÕES**](https://github.com/lupi-org-br/lupinho/issues/12)  

## ✨ Recursos

- 🌐 **Funciona no navegador** — Roda em WebAssembly, necessário poucas instalações e configurações
- 📜 **Scripts em Lua** — Escreva seus jogos em Lua de forma simples e fácil de aprender
- 🎨 **Gráficos 2D** — Desenhe texto, linhas, retângulos, círculos e triângulos
- 🖼️ **Sprites e Tiles** — Carregue e desenhe spritesheets com suporte a inversão (flip)
- 🗺️ **Sistema de Mapas (Tilemap)** — Desenhe grandes mapas com rolagem de câmera (scrolling)
- 🎮 **Controle e Teclado** — Suporte de entrada tanto para controles quanto para teclado
- ⚡ **60 FPS** — Jogabilidade fluida a 60 quadros por segundo
- 🔧 **Feito em Raylib** — Construído sobre a leve biblioteca gráfica Raylib

## 🛠️ Tecnologias Utilizadas

| Componente | Tecnologia |
|------------|------------|
| Linguagem | C99 |
| Compilador | Emscripten (emcc) |
| Gráficos | Raylib |
| Scripting | Lua 5.4 |
| Plataforma | WebAssembly |
| Tela | 480×270 px @ 60 FPS |

## 📁 Estrutura do Projeto

```
lupinho/
├── dist/                    # Saída do WebAssembly Compilado
│   ├── game.html            # Estes são os arquivos que você deve copiar para lupinho/site/dist/
│   ├── game.js
│   ├── game.wasm
│   └── game.data
├── site/
|   ├── index.html
│   └── dist/
│       └── .keep            # Cole a Saída do WebAssembly Compilado aqui
├── src/
│   ├── main.c               # Ponto de entrada principal
│   ├── lua_api.c            # Associações (bindings) de Lua para a API ui.*
│   ├── ui.c/h               # Sistema de renderização e buffer de quadro
│   ├── types.h              # Definições de tipos de itens de desenho
│   ├── font.h               # Fonte bitmap embutida
│   ├── Makefile
│   └── libs/
│       ├── lua-web/         # Lua compilado para WebAssembly
│       └── raylib-web/      # Raylib compilado para WebAssembly
├── game-example/            # Diretório de jogo de exemplo
│   ├── game.lua             # Ponto de entrada do jogo
│   ├── palette.lua          # Definição da paleta de cores - Gerado pelo lupi-codec
│   ├── lupi_manifest.txt    # Arquivo de metadados para o Lupi - Gerado pelo lupi-codec
│   ├── img/                 # Sprites bitmap
│   └── map/                 # Dados do mapa de tiles
└── README.md                # Este Leia-me
└── README_en.md             # Leia-me em inglês
```

## 🚀 Primeiros Passos

### Pré-requisitos

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) instalado e ativado
- Caso use Sistema Linux, procure por emscripten na loja de sua distro.
- Se usa Windows, recomendo instalar o Ubuntu via WSL (Subsistema Linux para windows) e realizar os procedimentos lá.

### Compilando

```bash
cd src

# Versão de desenvolvimento
make web

# Versão de produção (otimizada, sem depuração)
make production
```

### Executando

Após compilar, copie o conteúdo de **`lupinho/dist/`** e cole em **`lupinho/site/dist/`**.  
Então volte a **`lupinho/site/`** e crie um servidor https local nesta past, abaixo temos exemplos de Python e Node.js.

#### Usando Node.js
```bash
cd /site
npx serve
```
#### Usando Python3
```bash
cd /site
python3 -m http.server 8080
```

Em seguida, abra `http://localhost:8080` no seu navegador.

## 📖 API Lua

Os jogos são escritos em Lua.  
Seu script deve definir a função `update()`, que é chamada a cada quadro (60 vezes por segundo).

### `update()`
Chamada a cada quadro. Atualize a lógica do seu jogo e faça os desenhos aqui.

### Sistema de Paleta de Cores

O Lupinho usa um sistema de cores baseado em paleta com 256 cores no formato BGR555 (5 bits por canal, Big Endian).

| Função | Descrição |
|--------|-----------|
| `ui.palset(index, bgr555)` | Define uma cor da paleta no índice especificado (0-255) |

### Funções de Desenho

Todas as funções de desenho usam índices da paleta (0-255) para as cores:

| Função | Descrição |
|--------|-----------|
| `ui.rect(x1, y1, x2, y2, color)` | Desenha o contorno de um retângulo |
| `ui.rectfill(x1, y1, x2, y2, color)` | Desenha um retângulo preenchido |
| `ui.draw_rect(x, y, w, h, filled, color)` | Desenha um retângulo (preenchido ou contorno) |
| `ui.circfill(x, y, radius, color)` | Desenha um círculo preenchido |
| `ui.draw_circle(cx, cy, r, filled, color, border, border_color)` | Desenha um círculo com borda opcional |
| `ui.trisfill(x1, y1, x2, y2, x3, y3, color)` | Desenha um triângulo preenchido |
| `ui.line(x1, y1, x2, y2, color)` | Desenha uma linha |
| `ui.print(text, x, y, color)` | Desenha texto usando a fonte bitmap |

### Funções de Tela

| Função | Descrição |
|--------|-----------|
| `ui.cls(color)` | Limpa a tela com a cor da paleta |
| `ui.camera(x, y)` | Define o deslocamento da câmera; `ui.camera()` reseta |
| `ui.clip(x, y, w, h)` | Define a região de corte (clip); `ui.clip()` reseta |
| `ui.fillp(b1, b2, ...)` | Define o padrão de preenchimento 8x8 (1 byte por linha) |

### Sprites e Tiles

| Função | Descrição |
|--------|-----------|
| `ui.spr(sprite_table, x, y, flipped)` | Desenha um sprite |
| `ui.tile(tileset_table, tile_index, x, y)` | Desenha um tile (o bit 10 do índice inverte horizontalmente) |

### Funções de Mapa

| Função | Descrição |
|--------|-----------|
| `ui.map(layer_table, cam_x, cam_y)` | Desenha uma camada do mapa de tiles |

### Funções de Entrada

| Função | Descrição |
|--------|-----------|
| `ui.btn(button, pad)` | Verifica se o botão está pressionado (controle ou teclado) |
| `ui.btnp(button, pad)` | Verifica se o botão acabou de ser pressionado |

Constantes de botões: `UP`, `DOWN`, `LEFT`, `RIGHT`, `BTN_Z`, `BTN_Q`, `BTN_E`, `BTN_F`, `BTN_G`

### Funções Utilitárias

| Função | Descrição |
|--------|-----------|
| `ui.log(message)` | Imprime no console |
| `ui.mid(a, b, c)` | Retorna o valor do meio entre três números |

### Jogo de Exemplo

```lua
-- Defina sua paleta
-- O primeiro índice (cor) sempre será transparente (0x0000).
Palette = {
    [1] = 0x0000,
    [2] = 0x1516,
    [3] = 0x25B4,
    [4] = 0x20A6,
    [5] = 0x1DFD,
    [6] = 0x46FE,
    [7] = 0x7FFF,
    [8] = 0x2532
}

-- Faz a Contagem e define as cores da paleta
for i = 1, #Palette do
-- Ajusta o índice do Lua (começa em 1) para O índice da API Raylib (começa em 0)
    ui.palset(i - 1, Palette[i])
end

-- Variáveis do jogo
x = 200
t = 0

function update()
    t = t + 0.05
    y = math.sin(t) * 25

    ui.print("Bem-vindo ao Lupi!", 280, 180 + math.floor(y))
    ui.rect(50, 50, 130, 130, 1)
    ui.circfill(200, 100, 20, 3)
    ui.trisfill(20, 250, 100, 250, 55, 350, 4)
    
    -- Exemplo de entrada
    if ui.btnp(BTN_Z) then
        ui.log("Botão Z pressionado!")
    end
end
```

## 🧹 Limpeza
Este comando irá remover os arquivos compilados.
```bash
cd /src
make clean
```

## 🤝 Contribuindo
Encorajamos contribuições!  
No entanto, **por favor, abra uma issue antes de enviar um PR** para que possamos discutir as mudanças.  
As issues podem ser escritas em **português, espanhol ou inglês**.

Junte-se ao nosso Discord!  
### 💬 [Lupi](lupi.api.br/l/discord)

## 📝 Licença

Este projeto é de código aberto.

---
<div align="center">
  
**Feito com ❤️ no Brasil 🇧🇷**  
**Revisado e Atualizado por : [Hiriko](https://github.com/Hirikko) 🩷 e [Felipowsky](https://github.com/felipowsky) 🧡**

</div>
