import heapq, itertools, random, re, subprocess, sys

INF = float("inf")
random.seed(2026)


def escribir(nombre, ciudades, vuelos):
    with open(nombre, "w") as f:
        f.write("\n".join(ciudades) + "\n#\n")
        for a, b, c in vuelos:
            f.write(f"{ciudades[a]} {ciudades[b]} {c}\n")


def red_aleatoria(n, m, cmin, cmax, conexa=True):
    pares = set()
    if conexa:
        for i in range(1, n):
            pares.add((random.randrange(i), i))
    while len(pares) < m:
        a, b = random.sample(range(n), 2)
        pares.add((min(a, b), max(a, b)))
    return [(a, b, random.randint(cmin, cmax)) for a, b in sorted(pares)]


def matriz(n, vuelos):
    w = [dict() for _ in range(n)]
    for a, b, c in vuelos:
        if c < w[a].get(b, INF):
            w[a][b] = w[b][a] = c
    return w


def dijkstra(w, s):
    d = [INF] * len(w)
    d[s] = 0
    pq = [(0, s)]
    while pq:
        du, u = heapq.heappop(pq)
        if du > d[u]:
            continue
        for v, c in w[u].items():
            if du + c < d[v]:
                d[v] = du + c
                heapq.heappush(pq, (d[v], v))
    return d


def por_capas(w, s, capas):
    d = [INF] * len(w)
    d[s] = 0
    salida = []
    for _ in range(capas):
        nd = d[:]
        for u in range(len(w)):
            if d[u] < INF:
                for v, c in w[u].items():
                    if d[u] + c < nd[v]:
                        nd[v] = d[u] + c
        d = nd
        salida.append(d)
    return salida


def correr(archivo, comandos):
    entrada = archivo + "\n" + "".join(c + "\n" for c in comandos) + "0\n"
    out = subprocess.run(["./vuelos"], input=entrada, capture_output=True, text=True).stdout
    return out.split("Opcion > ")[1:]


def dinero(s):
    return float(s.replace("$", ""))


def revisar_ruta(w, ciudades, txt, a, b):
    idx = {c: i for i, c in enumerate(ciudades)}
    p = [idx[x.strip()] for x in txt.split(" -> ")]
    assert p[0] == a and p[-1] == b and len(set(p)) == len(p), "ruta invalida"
    return sum(w[u][v] for u, v in zip(p, p[1:])), len(p) - 2


class Conteo:
    def __init__(self):
        self.ok = {}
    def suma(self, clave, k=1):
        self.ok[clave] = self.ok.get(clave, 0) + k


def prueba_grande(cuenta):
    n, m = 1000, 5000
    ciudades = [f"Ciudad{i:04d}" for i in range(n)]
    vuelos = red_aleatoria(n, m, 300, 9000)
    escribir("masivo.txt", ciudades, vuelos)
    w = matriz(n, vuelos)
    comandos, plan = [], []

    def par():
        return random.sample(range(n), 2)

    for _ in range(400):
        a, b = par()
        comandos += ["1", ciudades[a], ciudades[b]]
        plan.append(("ruta", a, b))
    for _ in range(300):
        a, b = par()
        e = random.randint(0, 5)
        comandos += ["4", ciudades[a], ciudades[b], str(e)]
        plan.append(("limite", a, b, e))
    for _ in range(100):
        a, b = par()
        comandos += ["5", ciudades[a], ciudades[b]]
        plan.append(("alt", a, b))
    for _ in range(5):
        a = random.randrange(n)
        comandos += ["2", ciudades[a]]
        plan.append(("desde", a))
    for _ in range(200):
        a, b = par()
        c = random.randint(50, 3000)
        comandos += ["6", ciudades[a], ciudades[b], str(c)]
        plan.append(("alta", a, b, c))
        x, y = par()
        if random.random() < 0.5:
            x = a
        comandos += ["1", ciudades[x], ciudades[y]]
        plan.append(("ruta", x, y))
    comandos += ["7"]
    plan.append(("comparar",))

    trozos = correr("masivo.txt", comandos)
    assert len(trozos) == len(plan) + 1, "numero de respuestas inesperado"
    for paso, t in zip(plan, trozos):
        if paso[0] == "ruta":
            _, a, b = paso
            d = dijkstra(w, a)[b]
            costo = dinero(re.search(r"Costo total (\S+)", t).group(1))
            ruta = re.search(r"Ruta\s+(.+)", t).group(1)
            esc = int(re.search(r"Escalas\s+(\d+)", t).group(1))
            suma, e2 = revisar_ruta(w, ciudades, ruta, a, b)
            assert costo == d == suma and esc == e2, ("ruta", a, b, costo, d)
            cuenta.suma("consulta 1, ruta mas economica")
        elif paso[0] == "limite":
            _, a, b, e = paso
            capas = por_capas(w, a, e + 1)
            filas = re.findall(r"hasta (\d+) escalas\s+(sin ruta|\S+)(?:\s+extra\s+\S+\s+(.+))?", t)
            assert filas, t
            for k, valor, ruta in filas:
                esperado = capas[int(k)][b]
                if valor == "sin ruta":
                    assert esperado == INF
                else:
                    suma, esc = revisar_ruta(w, ciudades, ruta, a, b)
                    assert dinero(valor) == esperado == suma and esc <= int(k), ("limite", a, b, k)
                cuenta.suma("consulta 4, maximo de escalas (filas)")
        elif paso[0] == "alt":
            _, a, b = paso
            d = dijkstra(w, a)[b]
            mejor = dinero(re.search(r"costo (\S+) \(", t).group(1))
            assert mejor == d
            vistas = set()
            for ruta in re.findall(r"^  (\S.*?)   escalas \d+$", t, re.M):
                ruta = re.sub(r"^\$\S+ \(extra \S+\)\s+", "", ruta)
                suma, _ = revisar_ruta(w, ciudades, ruta, a, b)
                assert suma >= d and ruta not in vistas
                vistas.add(ruta)
            cuenta.suma("consulta 5, empates y alternativa (validez)")
        elif paso[0] == "desde":
            a = paso[1]
            d = dijkstra(w, a)
            filas = re.findall(r"(Ciudad\d{4})\s+(\$\S+)\s+escalas", t)
            assert len(filas) == sum(1 for x in d if x < INF) - 1
            for nombre, valor in filas:
                assert dinero(valor) == d[int(nombre[6:])]
                cuenta.suma("consulta 2, destinos alcanzables (filas)")
        elif paso[0] == "alta":
            _, a, b, c = paso
            if c < w[a].get(b, INF):
                w[a][b] = w[b][a] = c
                assert "Vuelo registrado" in t
            else:
                assert "no cambia nada" in t
            cuenta.suma("consulta 6, altas de vuelo")
        else:
            assert "Floyd-Warshall y Dijkstra coinciden" in t and "recalculo completo coinciden" in t, t
            cuenta.suma("consulta 7, un millon de pares Floyd contra Dijkstra")
            print(t.split("\n\n")[0])


def prueba_chica(cuenta, redes=150):
    n = 8
    ciudades = [f"N{i}" for i in range(n)]
    for _ in range(redes):
        vuelos = red_aleatoria(n, random.randint(6, 16), 1, 6, conexa=False)
        escribir("chica.txt", ciudades, vuelos)
        w = matriz(n, vuelos)
        pares = [(a, b) for a in range(n) for b in range(n) if a != b]
        random.shuffle(pares)
        pares = pares[:12]
        comandos = []
        for a, b in pares:
            comandos += ["5", ciudades[a], ciudades[b], "1", ciudades[a], ciudades[b]]
        trozos = correr("chica.txt", comandos)
        for k, (a, b) in enumerate(pares):
            t5, t1 = trozos[2 * k], trozos[2 * k + 1]
            todas = []
            otros = [x for x in range(n) if x not in (a, b)]
            for largo in range(len(otros) + 1):
                for medio in itertools.permutations(otros, largo):
                    p = (a,) + medio + (b,)
                    if all(v in w[u] for u, v in zip(p, p[1:])):
                        todas.append((sum(w[u][v] for u, v in zip(p, p[1:])), len(p) - 2))
            if not todas:
                assert "No hay forma" in t5 and "No hay forma" in t1
                cuenta.suma("fuerza bruta, pares sin conexion")
                continue
            todas.sort()
            mejor = todas[0][0]
            empates = sum(1 for c, _ in todas if c == mejor)
            mayores = [c for c, _ in todas if c > mejor]
            assert int(re.search(r"Escalas\s+(\d+)", t1).group(1)) == todas[0][1]
            cuenta.suma("fuerza bruta, minimo de escalas en empates")
            if empates >= 15:
                continue
            m = re.search(r"costo (\S+) \((\d+)\)", t5)
            assert dinero(m.group(1)) == mejor and int(m.group(2)) == empates, (vuelos, a, b)
            cuenta.suma("fuerza bruta, numero de rutas empatadas")
            sig = re.search(r"Siguiente alternativa\n  (\S+) ", t5)
            if empates < 14:
                if mayores:
                    assert sig and dinero(sig.group(1)) == mayores[0], (vuelos, a, b)
                else:
                    assert "No existe ninguna otra ruta" in t5
                cuenta.suma("fuerza bruta, siguiente alternativa")


cuenta = Conteo()
prueba_grande(cuenta)
prueba_chica(cuenta)
total = 0
print("\nVerificaciones aprobadas")
for clave, k in cuenta.ok.items():
    print(f"  {k:6d}  {clave}")
    total += k
print(f"  {total:6d}  total")
