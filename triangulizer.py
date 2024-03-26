import math

distances = [
    ['bia', 'ols', 210],
    ['bia', 'waw', 132],
    ['bia', 'lub', 226],
    ['bia', 'byd', 306],
    ['bia', 'gda', 289],
    ['bia', 'ldz', 209],
    ['bia', 'poz', 306],
    ['bia', 'szc', 448],
    ['bia', 'gwl', 391],
    ['bia', 'wro', 333],
    ['bia', 'opo', 343],
    ['bia', 'kat', 309],
    ['bia', 'kie', 247],
    ['bia', 'kra', 327],
    ['bia', 'rze', 320],
    ['ols', 'waw', 161],
    ['ols', 'lub', 263],
    ['ols', 'byd', 187],
    ['ols', 'gda', 114],
    ['ols', 'ldz', 211],
    ['ols', 'poz', 268],
    ['ols', 'szc', 367],
    ['ols', 'gwl', 353],
    ['ols', 'wro', 335],
    ['ols', 'opo', 350],
    ['ols', 'kat', 316],
    ['ols', 'kie', 273],
    ['ols', 'kra', 353],
    ['ols', 'rze', 355],
    ['waw', 'lub', 118],
    ['waw', 'byd', 200],
    ['waw', 'gda', 229],
    ['waw', 'ldz', 104],
    ['waw', 'poz', 198],
    ['waw', 'szc', 340],
    ['waw', 'gwl', 283],
    ['waw', 'wro', 225],
    ['waw', 'opo', 228],
    ['waw', 'kat', 193],
    ['waw', 'kie', 128],
    ['waw', 'kra', 209],
    ['waw', 'rze', 207],
    ['lub', 'byd', 286],
    ['lub', 'gda', 341],
    ['lub', 'ldz', 189],
    ['lub', 'poz', 287],
    ['lub', 'szc', 429],
    ['lub', 'gwl', 372],
    ['lub', 'wro', 314],
    ['lub', 'opo', 319],
    ['lub', 'kat', 248],
    ['lub', 'kie', 148],
    ['lub', 'kra', 211],
    ['lub', 'rze', 114],
    ['byd', 'gda', 116],
    ['byd', 'ldz', 155],
    ['byd', 'poz', 97] ,   
    ['byd', 'szc', 219],
    ['byd', 'gwl', 183],
    ['byd', 'wro', 197],
    ['byd', 'opo', 254],
    ['byd', 'kat', 258],
    ['byd', 'kie', 262],
    ['byd', 'kra', 312],
    ['byd', 'rze', 380],
    ['gda', 'ldz', 206],
    ['gda', 'poz', 197],
    ['gda', 'szc', 262],
    ['gda', 'gwl', 281],
    ['gda', 'wro', 295],
    ['gda', 'opo', 345],
    ['gda', 'kat', 311],
    ['gda', 'kie', 314],
    ['gda', 'kra', 365],
    ['gda', 'rze', 422],
    ['ldz', 'poz', 143],
    ['ldz', 'szc', 282],
    ['ldz', 'gwl', 226],
    ['ldz', 'wro', 145],
    ['ldz', 'opo', 167],
    ['ldz', 'kat', 137],
    ['ldz', 'kie', 139],
    ['ldz', 'kra', 193],
    ['ldz', 'rze', 274],
    ['poz', 'szc', 168],
    ['poz', 'gwl', 109],
    ['poz', 'wro', 127],
    ['poz', 'opo', 182],
    ['poz', 'kat', 232],
    ['poz', 'kie', 260],
    ['poz', 'kra', 286],
    ['poz', 'rze', 368],
    ['szc', 'gwl', 73] ,   
    ['szc', 'wro', 247],
    ['szc', 'opo', 289],
    ['szc', 'kat', 337],
    ['szc', 'kie', 402],
    ['szc', 'kra', 391],
    ['szc', 'rze', 473],
    ['gwl', 'wro', 187],
    ['gwl', 'opo', 229],
    ['gwl', 'kat', 278],
    ['gwl', 'kie', 342],
    ['gwl', 'kra', 332],
    ['gwl', 'rze', 414],
    ['wro', 'opo', 81],
    ['wro', 'kat', 129],
    ['wro', 'kie', 252],
    ['wro', 'kra', 183],
    ['wro', 'rze', 265],
    ['opo', 'kat', 83],
    ['opo', 'kie', 210],
    ['opo', 'kra', 138],
    ['opo', 'rze', 220],
    ['kat', 'kie', 132],
    ['kat', 'kra', 67],
    ['kat', 'rze', 148],
    ['kie', 'kra', 100],
    ['kie', 'rze', 145],
    ['kra', 'rze', 112], 
]

unique_towns = []

# extract all towns to a single array
for entry in distances:
    unique_towns.append(entry[0])
    unique_towns.append(entry[1])

# filter to unique town only
unique_towns = list(set(unique_towns))

print('towns: ', unique_towns)

bia_ols_dist_list = unique_towns

# remove targets
bia_ols_dist_list.remove('bia')
bia_ols_dist_list.remove('ols')

def find_dist(src: str, dest: str):
    for dist in distances:
        if dist[0] == src and dist[1] == dest:
            return dist[2]
        if dist[1] == src and dist[0] == dest:
            return dist[2]
    pass

def to_dist_list(dest: str):
    return [
        dest,
        find_dist('bia', dest),
        find_dist('ols', dest)
    ]

bia_ols_dist_list = list(map(to_dist_list, bia_ols_dist_list))

print('trig a & b dists: ', bia_ols_dist_list)

def angle_calculator(a: int, b: int, base: int = 210):
    # basic side side side rule calculation,
    # returns all angles
    A = math.degrees(math.acos((b**2 + base**2 - a**2) / (2 * b * base)))
    B = math.degrees(math.acos((a**2 + base**2 - b**2) / (2 * a * base)))
    C = 180 - A - B
    return A, B, C

def angle_to_coord(a, b, c, base):
    a_x, a_y = 0, 0
    b_x, b_y = base, 0
    c_x = base * math.cos(math.radians(b))
    c_y = base * math.sin(math.radians(b))
    # C is the dynamic one, a & b are static
    return c_x, c_y

idx = 3
for dist in bia_ols_dist_list:
    base = 210  # dist from bia to ols
    ang_a, ang_b, ang_c = angle_calculator(dist[1], base, dist[2])
    coords = angle_to_coord(ang_a, ang_b, ang_c, base)
    print(idx, coords[0], coords[1], dist[0])
    idx += 1