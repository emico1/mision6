#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

const double INF = std::numeric_limits<double>::infinity();
const double EPS = 1e-9;

using Lista = std::vector<std::vector<std::pair<int, double>>>;
using Camino = std::vector<int>;

struct Red {
    std::vector<std::string> ciudades;
    std::vector<std::pair<std::string, int>> indice;
    std::vector<std::vector<double>> directo;
    std::vector<std::vector<double>> costo;
    std::vector<std::vector<int>> sig;
    std::vector<std::vector<int>> tramos;
    Lista ady;
};

// O(L)
std::string limpiar(const std::string& s) {
    const char* blancos = " \t\r\n";
    std::size_t a = s.find_first_not_of(blancos);
    if (a == std::string::npos) return "";
    std::size_t b = s.find_last_not_of(blancos);
    return s.substr(a, b - a + 1);
}

// O(L)
std::string minusculas(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// O(log n)
int buscar(const Red& r, const std::string& nombre) {
    std::string clave = minusculas(limpiar(nombre));
    auto it = std::lower_bound(r.indice.begin(), r.indice.end(), std::make_pair(clave, -1));
    if (it != r.indice.end() && it->first == clave) return it->second;
    return -1;
}

// O(L log n)
bool leerVuelo(const Red& r, const std::string& linea, int& a, int& b, double& c) {
    std::size_t p = linea.find_last_of(" \t");
    if (p == std::string::npos) return false;
    std::istringstream num(linea.substr(p + 1));
    if (!(num >> c) || c < 0) return false;
    std::string resto = limpiar(linea.substr(0, p));
    for (std::size_t i = 0; i < resto.size(); i++) {
        if (resto[i] != ' ' && resto[i] != '\t') continue;
        a = buscar(r, resto.substr(0, i));
        b = buscar(r, resto.substr(i + 1));
        if (a != -1 && b != -1) return true;
    }
    return false;
}

// O(n^2 + m L log n)
bool cargar(const std::string& archivo, Red& r) {
    std::ifstream in(archivo);
    if (!in) return false;
    std::string linea;
    while (std::getline(in, linea)) {
        linea = limpiar(linea);
        if (linea == "#") break;
        if (!linea.empty()) r.ciudades.push_back(linea);
    }
    int n = static_cast<int>(r.ciudades.size());
    for (int i = 0; i < n; i++) r.indice.push_back({minusculas(r.ciudades[i]), i});
    std::sort(r.indice.begin(), r.indice.end());

    r.directo.assign(n, std::vector<double>(n, INF));
    for (int i = 0; i < n; i++) r.directo[i][i] = 0;
    int numLinea = n + 1;
    while (std::getline(in, linea)) {
        numLinea++;
        linea = limpiar(linea);
        if (linea.empty()) continue;
        int a, b;
        double c;
        if (!leerVuelo(r, linea, a, b, c)) {
            std::cout << "Aviso, linea " << numLinea << " ignorada (" << linea << ")\n";
            continue;
        }
        if (a == b) continue;
        r.directo[a][b] = std::min(r.directo[a][b], c);
        r.directo[b][a] = std::min(r.directo[b][a], c);
    }
    r.ady.assign(n, {});
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && r.directo[i][j] < INF) r.ady[i].push_back({j, r.directo[i][j]});
    return n > 0;
}

// O(n^3) tiempo y O(n^2) espacio
void floyd(Red& r) {
    int n = static_cast<int>(r.ciudades.size());
    r.costo = r.directo;
    r.sig.assign(n, std::vector<int>(n, -1));
    r.tramos.assign(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && r.directo[i][j] < INF) {
                r.sig[i][j] = j;
                r.tramos[i][j] = 1;
            }
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++) {
            if (r.costo[i][k] == INF) continue;
            for (int j = 0; j < n; j++) {
                if (r.costo[k][j] == INF) continue;
                double c = r.costo[i][k] + r.costo[k][j];
                int t = r.tramos[i][k] + r.tramos[k][j];
                if (c < r.costo[i][j] || (c == r.costo[i][j] && t < r.tramos[i][j])) {
                    r.costo[i][j] = c;
                    r.tramos[i][j] = t;
                    r.sig[i][j] = r.sig[i][k];
                }
            }
        }
}

// O(n^2) tiempo y O(n) espacio extra
int agregarVuelo(Red& r, int a, int b, double c) {
    int n = static_cast<int>(r.ciudades.size());
    bool existia = r.directo[a][b] < INF;
    r.directo[a][b] = r.directo[b][a] = c;
    if (existia) {
        for (auto& e : r.ady[a]) if (e.first == b) e.second = c;
        for (auto& e : r.ady[b]) if (e.first == a) e.second = c;
    } else {
        r.ady[a].push_back({b, c});
        r.ady[b].push_back({a, c});
    }
    std::vector<double> ca(n), cb(n);
    std::vector<int> ta(n), tb(n), sa(n), sb(n);
    for (int i = 0; i < n; i++) {
        ca[i] = r.costo[i][a];
        cb[i] = r.costo[i][b];
        ta[i] = r.tramos[i][a];
        tb[i] = r.tramos[i][b];
        sa[i] = r.sig[i][a];
        sb[i] = r.sig[i][b];
    }
    int mejoras = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            double antes = r.costo[i][j];
            for (int lado = 0; lado < 2; lado++) {
                double ci = lado == 0 ? ca[i] : cb[i], cj = lado == 0 ? cb[j] : ca[j];
                if (ci == INF || cj == INF) continue;
                double nc = ci + c + cj;
                int nt = (lado == 0 ? ta[i] + tb[j] : tb[i] + ta[j]) + 1;
                int ns = lado == 0 ? (i == a ? b : sa[i]) : (i == b ? a : sb[i]);
                if (nc < r.costo[i][j] || (nc == r.costo[i][j] && nt < r.tramos[i][j])) {
                    r.costo[i][j] = nc;
                    r.tramos[i][j] = nt;
                    r.sig[i][j] = ns;
                }
            }
            if (i < j && r.costo[i][j] < antes) mejoras++;
        }
    return mejoras;
}

// O(longitud de la ruta)
Camino camino(const Red& r, int a, int b) {
    Camino p{a};
    while (a != b) {
        a = r.sig[a][b];
        p.push_back(a);
    }
    return p;
}

// O(longitud de la ruta)
std::string texto(const Red& r, const Camino& p) {
    std::string s;
    for (std::size_t i = 0; i < p.size(); i++) s += (i ? " -> " : "") + r.ciudades[p[i]];
    return s;
}

// O((n + m) log n)
std::vector<double> dijkstra(const Lista& ady, int origen, const std::vector<char>& bloq,
                             const std::vector<char>& noIr, std::vector<int>& padre) {
    std::vector<double> d(ady.size(), INF);
    padre.assign(ady.size(), -1);
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>,
                        std::greater<std::pair<double, int>>> pq;
    d[origen] = 0;
    pq.push({0, origen});
    while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        if (du > d[u]) continue;
        for (auto [v, w] : ady[u]) {
            if (!bloq.empty() && bloq[v]) continue;
            if (u == origen && !noIr.empty() && noIr[v]) continue;
            if (du + w < d[v]) {
                d[v] = du + w;
                padre[v] = u;
                pq.push({d[v], v});
            }
        }
    }
    return d;
}

// O(R L (n + m) log n), R = rutas generadas, L = ciudades por ruta
std::vector<std::pair<double, Camino>> mejoresRutas(const Red& r, int a, int b, int tope) {
    int n = static_cast<int>(r.ciudades.size());
    std::vector<std::pair<double, Camino>> A;
    A.push_back({r.costo[a][b], camino(r, a, b)});
    std::set<std::pair<double, Camino>> B;
    std::vector<int> padre;
    while (static_cast<int>(A.size()) < tope) {
        Camino p = A.back().second;
        std::vector<char> bloq(n, 0);
        double costoRaiz = 0;
        for (std::size_t i = 0; i + 1 < p.size(); i++) {
            std::vector<char> noIr(n, 0);
            for (const auto& q : A)
                if (q.second.size() > i + 1 && std::equal(p.begin(), p.begin() + i + 1, q.second.begin()))
                    noIr[q.second[i + 1]] = 1;
            std::vector<double> d = dijkstra(r.ady, p[i], bloq, noIr, padre);
            if (d[b] < INF) {
                Camino cola;
                for (int v = b; v != p[i]; v = padre[v]) cola.push_back(v);
                Camino nuevo(p.begin(), p.begin() + i + 1);
                nuevo.insert(nuevo.end(), cola.rbegin(), cola.rend());
                B.insert({costoRaiz + d[b], nuevo});
            }
            bloq[p[i]] = 1;
            costoRaiz += r.directo[p[i]][p[i + 1]];
        }
        if (B.empty()) break;
        A.push_back(*B.begin());
        B.erase(B.begin());
        if (A.back().first > A[0].first + EPS) break;
    }
    return A;
}

std::string dinero(double c) {
    std::ostringstream o;
    o << "$" << std::fixed << std::setprecision(2) << c;
    return o.str();
}

int pedirCiudad(const Red& r, const std::string& mensaje) {
    std::string s;
    while (true) {
        std::cout << mensaje << " > ";
        if (!std::getline(std::cin, s)) return -1;
        int id = buscar(r, s);
        if (id != -1) return id;
        std::cout << "Esa ciudad no esta en el archivo, intenta de nuevo\n";
    }
}

double pedirNumero(const std::string& mensaje, bool entero) {
    std::string s;
    while (true) {
        std::cout << mensaje << " > ";
        if (!std::getline(std::cin, s)) return -1;
        std::istringstream in(s);
        double x;
        if (in >> x && x >= 0 && (!entero || x == std::floor(x))) return x;
        std::cout << (entero ? "Escribe un entero mayor o igual a cero\n" : "Escribe un numero mayor o igual a cero\n");
    }
}

// O(longitud de la ruta)
void rutaEconomica(const Red& r) {
    int a = pedirCiudad(r, "Ciudad origen");
    int b = pedirCiudad(r, "Ciudad destino");
    if (a == -1 || b == -1) return;
    if (a == b) {
        std::cout << "Origen y destino son la misma ciudad, costo " << dinero(0) << "\n";
    } else if (r.costo[a][b] == INF) {
        std::cout << "No hay forma de volar de " << r.ciudades[a] << " a " << r.ciudades[b] << "\n";
    } else {
        std::cout << "Costo total " << dinero(r.costo[a][b]) << "\n";
        std::cout << "Ruta        " << texto(r, camino(r, a, b)) << "\n";
        std::cout << "Escalas     " << r.tramos[a][b] - 1 << "\n";
    }
}

// O(n)
void alcanzables(const Red& r) {
    int a = pedirCiudad(r, "Ciudad origen");
    if (a == -1) return;
    int n = static_cast<int>(r.ciudades.size()), cuenta = 0;
    for (int j = 0; j < n; j++) {
        if (j == a || r.costo[a][j] == INF) continue;
        std::cout << "  " << std::left << std::setw(22) << r.ciudades[j] << std::right
                  << std::setw(12) << dinero(r.costo[a][j]) << "   escalas " << r.tramos[a][j] - 1 << "\n";
        cuenta++;
    }
    if (cuenta == 0) std::cout << "Desde " << r.ciudades[a] << " no sale ningun vuelo\n";
    else std::cout << cuenta << " ciudades alcanzables desde " << r.ciudades[a] << "\n";
}

// O(n^2) pares por O(n) de imprimir cada ruta
void porEscalas(const Red& r) {
    int e = static_cast<int>(pedirNumero("Numero de escalas", true));
    if (e < 0) return;
    int n = static_cast<int>(r.ciudades.size()), cuenta = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            if (r.costo[i][j] == INF || r.tramos[i][j] - 1 != e) continue;
            std::cout << "  " << std::setw(12) << dinero(r.costo[i][j]) << "   " << texto(r, camino(r, i, j)) << "\n";
            cuenta++;
        }
    std::cout << cuenta << " rutas optimas con " << e << " escalas\n";
}

// O(E (n + m)) tiempo y O(E n) espacio, E = maximo de escalas
void limiteEscalas(const Red& r) {
    int a = pedirCiudad(r, "Ciudad origen");
    int b = pedirCiudad(r, "Ciudad destino");
    if (a == -1 || b == -1) return;
    int E = static_cast<int>(pedirNumero("Maximo de escalas", true));
    if (E < 0) return;
    if (a == b) {
        std::cout << "Origen y destino son la misma ciudad\n";
        return;
    }
    if (r.costo[a][b] == INF) {
        std::cout << "No hay forma de volar de " << r.ciudades[a] << " a " << r.ciudades[b] << "\n";
        return;
    }
    int n = static_cast<int>(r.ciudades.size());
    int K = std::min(E, n - 2) + 1;
    std::vector<double> d(n, INF), nd;
    std::vector<std::vector<int>> par(K + 1, std::vector<int>(n, -1));
    d[a] = 0;
    bool optimo = false;
    for (int k = 1; k <= K && !optimo; k++) {
        nd = d;
        for (int u = 0; u < n; u++) {
            if (d[u] == INF) continue;
            for (auto [v, w] : r.ady[u])
                if (d[u] + w < nd[v]) {
                    nd[v] = d[u] + w;
                    par[k][v] = u;
                }
        }
        d = nd;
        std::cout << "  hasta " << k - 1 << " escalas   ";
        if (d[b] == INF) {
            std::cout << "sin ruta\n";
            continue;
        }
        Camino p;
        int v = b, kk = k;
        while (v != a) {
            if (par[kk][v] != -1) {
                p.push_back(v);
                v = par[kk][v];
            }
            kk--;
        }
        p.push_back(a);
        std::reverse(p.begin(), p.end());
        std::cout << std::setw(12) << dinero(d[b]) << "   extra " << std::setw(10) << dinero(d[b] - r.costo[a][b])
                  << "   " << texto(r, p) << "\n";
        optimo = d[b] <= r.costo[a][b] + EPS;
    }
    if (optimo) std::cout << "Con ese limite ya se alcanza la ruta mas barata posible\n";
    else std::cout << "Sin limite la mas barata cuesta " << dinero(r.costo[a][b]) << " con "
                   << r.tramos[a][b] - 1 << " escalas (" << texto(r, camino(r, a, b)) << ")\n";
}

// O(R L (n + m) log n), R = rutas mostradas, L = ciudades por ruta
void alternativas(const Red& r) {
    const int tope = 15;
    int a = pedirCiudad(r, "Ciudad origen");
    int b = pedirCiudad(r, "Ciudad destino");
    if (a == -1 || b == -1) return;
    if (a == b) {
        std::cout << "Origen y destino son la misma ciudad\n";
        return;
    }
    if (r.costo[a][b] == INF) {
        std::cout << "No hay forma de volar de " << r.ciudades[a] << " a " << r.ciudades[b] << "\n";
        return;
    }
    auto rutas = mejoresRutas(r, a, b, tope);
    double mejor = rutas[0].first;
    int empates = 0;
    for (const auto& x : rutas) if (x.first <= mejor + EPS) empates++;
    std::cout << "Rutas optimas con costo " << dinero(mejor) << " (" << empates << ")\n";
    for (int i = 0; i < empates; i++)
        std::cout << "  " << texto(r, rutas[i].second) << "   escalas " << rutas[i].second.size() - 2 << "\n";
    if (empates < static_cast<int>(rutas.size())) {
        const auto& x = rutas.back();
        std::cout << "Siguiente alternativa\n  " << dinero(x.first) << " (extra " << dinero(x.first - mejor) << ")   "
                  << texto(r, x.second) << "   escalas " << x.second.size() - 2 << "\n";
    } else if (empates == tope) {
        std::cout << "Hay mas rutas, solo se muestran las primeras " << tope << "\n";
    } else {
        std::cout << "No existe ninguna otra ruta entre esas ciudades\n";
    }
}

// O(n^2)
void altaVuelo(Red& r) {
    int a = pedirCiudad(r, "Ciudad origen");
    int b = pedirCiudad(r, "Ciudad destino");
    if (a == -1 || b == -1) return;
    double c = pedirNumero("Costo del vuelo", false);
    if (c < 0) return;
    if (a == b) {
        std::cout << "Origen y destino son la misma ciudad\n";
        return;
    }
    if (c >= r.directo[a][b]) {
        std::cout << "Ya existe ese vuelo por " << dinero(r.directo[a][b]) << ", no cambia nada\n";
        return;
    }
    auto t0 = std::chrono::steady_clock::now();
    int mejoras = agregarVuelo(r, a, b, c);
    auto t1 = std::chrono::steady_clock::now();
    std::cout << "Vuelo registrado, " << mejoras << " pares de ciudades ahora tienen una ruta mas barata\n";
    std::cout << "Actualizacion en " << std::fixed << std::setprecision(3)
              << std::chrono::duration<double, std::milli>(t1 - t0).count() << " milisegundos\n";
}

bool iguales(const std::vector<std::vector<double>>& x, const std::vector<std::vector<double>>& y) {
    for (std::size_t i = 0; i < x.size(); i++)
        for (std::size_t j = 0; j < x.size(); j++)
            if (!((x[i][j] == INF && y[i][j] == INF) || std::fabs(x[i][j] - y[i][j]) < EPS)) return false;
    return true;
}

void comparar(Red& r) {
    int n = static_cast<int>(r.ciudades.size());
    const int reps = static_cast<int>(std::max(1.0, std::min(2000.0, 2e8 / (double(n) * n * n))));
    using reloj = std::chrono::steady_clock;
    auto ms = [](reloj::time_point x, reloj::time_point y) { return std::chrono::duration<double, std::milli>(y - x).count(); };

    auto t0 = reloj::now();
    for (int k = 0; k < reps; k++) floyd(r);
    auto t1 = reloj::now();
    std::vector<std::vector<double>> d(n);
    std::vector<int> padre;
    for (int k = 0; k < reps; k++)
        for (int i = 0; i < n; i++) d[i] = dijkstra(r.ady, i, {}, {}, padre);
    auto t2 = reloj::now();
    bool okD = iguales(d, r.costo);

    Red copia = r;
    int a = 0, b = n - 1;
    double c = copia.directo[a][b] < INF ? copia.directo[a][b] / 2 : 100;
    auto t3 = reloj::now();
    int mejoras = n > 1 ? agregarVuelo(copia, a, b, c) : 0;
    auto t4 = reloj::now();
    auto incremental = copia.costo;
    auto incTramos = copia.tramos;
    floyd(copia);
    bool okI = iguales(incremental, copia.costo) && incTramos == copia.tramos;

    std::cout << "Ciudades " << n << ", promedio de " << reps << " repeticiones\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Floyd-Warshall      O(n^3)               " << ms(t0, t1) / reps << " ms\n";
    std::cout << "  Dijkstra x n        O(n (n + m) log n)   " << ms(t1, t2) / reps << " ms\n";
    std::cout << "  Alta incremental    O(n^2)               " << ms(t3, t4) << " ms (" << mejoras << " pares mejoran)\n";
    std::cout << (okD ? "  Floyd-Warshall y Dijkstra coinciden\n" : "  OJO, Floyd-Warshall y Dijkstra NO coinciden\n");
    std::cout << (okI ? "  Alta incremental y recalculo completo coinciden\n" : "  OJO, alta incremental y recalculo NO coinciden\n");
}

int main() {
    Red r;
    std::string archivo;
    std::cout << "Nombre del archivo de datos > ";
    std::getline(std::cin, archivo);
    if (!cargar(limpiar(archivo), r)) {
        std::cout << "No se pudo leer el archivo o no trae ciudades\n";
        return 1;
    }
    floyd(r);
    std::cout << r.ciudades.size() << " ciudades cargadas\n";

    std::string op;
    while (true) {
        std::cout << "\n1. Ruta mas economica entre dos ciudades\n"
                  << "2. Ciudades a las que se puede volar desde un origen\n"
                  << "3. Rutas optimas con cierto numero de escalas\n"
                  << "4. Ruta mas barata con un maximo de escalas\n"
                  << "5. Rutas empatadas y siguiente alternativa\n"
                  << "6. Dar de alta un vuelo nuevo\n"
                  << "7. Comparar Floyd-Warshall, Dijkstra y alta incremental\n"
                  << "0. Salir\n"
                  << "Opcion > ";
        if (!std::getline(std::cin, op)) break;
        op = limpiar(op);
        if (op == "1") rutaEconomica(r);
        else if (op == "2") alcanzables(r);
        else if (op == "3") porEscalas(r);
        else if (op == "4") limiteEscalas(r);
        else if (op == "5") alternativas(r);
        else if (op == "6") altaVuelo(r);
        else if (op == "7") comparar(r);
        else if (op == "0") break;
        else std::cout << "Opcion no valida\n";
    }
    return 0;
}