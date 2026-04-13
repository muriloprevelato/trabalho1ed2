#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svg.h"
#include "quadra.h"

#define MARCADOR_TAMANHO 6.0 // Delimitando tamanho do raio do círculo e dimensões do quadrado.
#define MARCADOR_MEIA 3.0 // Metade do tamanho do marcador.
#define FONTE_PADRAO 5 // Tamanho da fonte padrão em pixels.
#define FONTE_MARCADOR 3 // Tamanho da fonte para texto impresso no quadrado.

struct ArqSvg{
    FILE* arquivo;
};

ArqSvg* abrirSvg(const char *caminho){
    if(!caminho) return NULL;

    FILE* f = fopen(caminho, "w");
    if(!f) return NULL;

    ArqSvg *svg = (ArqSvg*) malloc(sizeof(ArqSvg));
    if(!svg) { fclose(f); return NULL; }

    svg->arquivo = f;
    // Obs: Usei o padrão da descrição geral!
    fprintf(f,
        "<svg xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
        "     xmlns=\"http://www.w3.org/2000/svg\"\n"
        "     version=\"1.1\">\n");
    
    return svg;
}

void fecharSvg(ArqSvg *svg){
    if(!svg) return;
    fprintf(svg->arquivo, "</svg>\n");
    fclose(svg->arquivo);
    free(svg);
}

int desenharQuadra(ArqSvg *svg, const Quadra *q){
    if(!svg || !q) return SVG_ERRO;

    double x = getQuadraX(q);
    double y = getQuadraY(q);
    double w = getQuadraW(q);
    double h = getQuadraH(q);
    double sw = getQuadraSw(q);
    const char *cfill = getQuadraCFill(q);
    const char *cstrk = getQuadraCStrk(q);

    // Quadra -> retângulo.
    fprintf(svg->arquivo,
        "  <rect style=\"fill:%s;fill-opacity:0.5;stroke:%s;stroke-width:%.1f\""
        " x=\"%.6f\" y=\"%.6f\" width=\"%.6f\" height=\"%.6f\" />\n",
        cfill, cstrk, sw, x, y, w, h);

    return SVG_OK;
}

int desenharMarcadorX(ArqSvg *svg, double x, double y){
    if(!svg) return SVG_ERRO;
    // X vermelho: duas linhas diagonais cruzadas 
    fprintf(svg->arquivo,
        "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
        " style=\"stroke:red;stroke-width:1\" />\n",
        x - MARCADOR_MEIA, y - MARCADOR_MEIA,
        x + MARCADOR_MEIA, y + MARCADOR_MEIA);

    fprintf(svg->arquivo,
        "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
        " style=\"stroke:red;stroke-width:1\" />\n",
        x + MARCADOR_MEIA, y - MARCADOR_MEIA,
        x - MARCADOR_MEIA, y + MARCADOR_MEIA);

    return SVG_OK;
}

int desenharMarcadorCruz(ArqSvg *svg, double x, double y){
    if(!svg) return SVG_ERRO;

    // Cruz vermelha: linha vertical e uma horizontal
    fprintf(svg->arquivo,
        "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
        " style=\"stroke:red;stroke-width:1\" />\n",
        x, y - MARCADOR_MEIA, x, y + MARCADOR_MEIA);
    
    fprintf(svg->arquivo,
        "  <line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\""
        " style=\"stroke:red;stroke-width:1\" />\n",
        x - MARCADOR_MEIA, y, x + MARCADOR_MEIA, y);

    return SVG_OK;
}

int desenharMarcadorCirculo(ArqSvg *svg, double x, double y){
    if(!svg) return SVG_ERRO;

    fprintf(svg->arquivo,
        "  <circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.1f\""
        " style=\"fill:black;fill-opacity:0.8\" />\n",
        x, y, MARCADOR_MEIA);

    return SVG_OK;
}

int desenharMarcadorQuadrado(ArqSvg *svg, double x, double y, const char *cpf){
    if(!svg) return SVG_ERRO;

    // Quadrado vermelho.
    fprintf(svg->arquivo,
        "  <rect x=\"%.2f\" y=\"%.2f\" width=\"%.1f\" height=\"%.1f\""
        " style=\"fill:red;fill-opacity:0.7\" />\n",
        x, y, MARCADOR_TAMANHO, MARCADOR_TAMANHO);
    
    // CPF em fonte minúscula centralizado.
    fprintf(svg->arquivo,
        "  <text style=\"font-size:%dpx;fill:white\" font-size=\"%d\""
        " x=\"%.2f\" y=\"%.2f\">%s</text>\n",
        FONTE_MARCADOR, FONTE_MARCADOR,
        x + 0.5, y + MARCADOR_TAMANHO - 0.5,
        cpf);

    return SVG_OK;
}

int desenharTextoCentro(ArqSvg *svg, double cx, double cy, const char *texto){
    if(!svg || !texto) return SVG_ERRO;

    fprintf(svg->arquivo,
        "  <text style=\"font-size:%dpx;text-anchor:middle;fill:black\""
        " font-size=\"%d\" x=\"%.2f\" y=\"%.2f\">%s</text>\n",
        FONTE_PADRAO, FONTE_PADRAO, cx, cy, texto);
    
    return SVG_OK;
}

int desenharTexto(ArqSvg *svg, double x, double y, const char *texto){
    if(!svg || !texto) return SVG_ERRO;

    fprintf(svg->arquivo,
        "  <text style=\"font-size:%dpx;fill:black\""
        " font-size=\"%d\" x=\"%.2f\" y=\"%.2f\">%s</text>\n",
        FONTE_PADRAO, FONTE_PADRAO, x, y, texto);
    
    return SVG_OK;
}