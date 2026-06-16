# AZIMOV JUMPGATE

AZIMOV JUMPGATE é um seletor de servidores para Overwatch 2 no Windows. O programa permite bloquear regiões específicas usando regras do Firewall do Windows, ajudando o jogador a evitar servidores indesejados e manter mais controle sobre a rota de conexão.

O projeto foi construído em C++ com interface em Dear ImGui, DirectX 11 e integração com APIs nativas do Windows.

## Recursos

- Bloqueio de regiões do Overwatch 2 pelo Firewall do Windows.
- Lista de servidores com ping estimado para cada região.
- Tunelamento por aplicativo, limitando os bloqueios ao executável do Overwatch.
- Botão para desbloquear rapidamente todos os servidores.
- Salvamento automático das regiões bloqueadas.
- Otimização de latência para priorizar o processo do Overwatch no Windows.
- Verificação de versão e estado remoto do aplicativo.
- Interface em português, feita para a comunidade brasileira de Overwatch.

## Regiões disponíveis

O Jumpgate inclui perfis para as seguintes regiões:

- Brasil
- EUA - Central
- EUA - Leste
- EUA - Oeste
- Finlândia
- Singapura
- Tóquio
- Arábia Saudita
- Coreia do Sul
- Austrália
- Taiwan
- Países Baixos

## Como funciona

O programa cria e gerencia uma regra própria no Firewall do Windows. Quando uma região é bloqueada, o Jumpgate grava os intervalos de IP correspondentes nessa regra.

Com o tunelamento ativado, o bloqueio é aplicado apenas ao executável do Overwatch, evitando interferência em outros jogos, navegadores ou aplicativos. Se o tunelamento for desativado, o bloqueio passa a ser global no dispositivo.

## Requisitos

- Windows 10 ou superior.
- Firewall do Windows ativado.
- Permissão de administrador.
- Visual Studio 2022 com suporte a C++ para compilar o projeto.
- Windows SDK 10.0.

## Como compilar

1. Abra o arquivo `jumpgate.sln` no Visual Studio 2022.
2. Selecione a configuração `Release`.
3. Selecione a plataforma `x64`.
4. Compile a solução.
5. O executável será gerado como `JUMPGATE.exe`.

Também é possível compilar via MSBuild:

```powershell
msbuild jumpgate.sln /p:Configuration=Release /p:Platform=x64
```

## Como usar

1. Execute o `JUMPGATE.exe` como administrador.
2. Escolha as regiões que deseja bloquear.
3. Mantenha o tunelamento ativado para aplicar os bloqueios apenas ao Overwatch.
4. Se não conseguir conectar a uma partida, use o botão de desbloqueio rápido.
5. Evite alterar bloqueios com o jogo aberto. Quando necessário, reinicie o Overwatch para aplicar as mudanças.

## Estrutura do projeto

```text
jumpgate/
├── jumpgate.sln
├── jumpgate/
│   ├── src/
│   │   ├── components/
│   │   ├── core/
│   │   └── util/
│   ├── assets/
│   └── vendor/
├── x64/
└── JUMPGATE/
```

## Tecnologias usadas

- C++
- Win32 API
- Windows Firewall API
- DirectX 11
- Dear ImGui
- Asio
- nlohmann/json
- stb_image

## Observações importantes

Este projeto altera regras do Firewall do Windows. Use com cuidado e sempre mantenha uma forma fácil de desbloquear os servidores caso tenha problemas de conexão.

O Jumpgate não é afiliado, endossado ou mantido pela Blizzard Entertainment. Overwatch é uma marca registrada de seus respectivos proprietários.

## Créditos

Desenvolvido a partir da source do Dropship, com melhorias, adaptação e tradução para o português brasileiro pela AZIMOV ESPORTS.

