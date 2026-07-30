import numpy as np
import matplotlib.pyplot as plt
import math

# --------------------
# CONVERTER PARAMETERS
# --------------------

Vin = 48
Vout = 6
Pout = 6
n = 6
D = 0.5

Vdsoff = 84        # 48V input + 6V*6 reflected 
Vdsoff_sec = 14      # 6V output + 48V/6 reflected

Iout = Pout / Vout

mu0 = 4 * np.pi * 1e-7  # The permeability of free space mu0 = 4π*10^-7 H/m
mur = 2200              # The relative permeability of ferrite mur (N87)
Ku         = 0.4        # Window fill factor (round wire, typical)
Jmax       = 6e6        # current density

dB_target  = 0.10       # T, target AC flux swing
eta_min    = 0.75       # reject design with efficiency worse than this
k_leak     = 0.015      # 1.5% of primary inductacne is assumed as leakage (manual winding)

# -------------------
# COMPONENT DATABASE
# -------------------

# Primary MOSFET parameters  (IPP600N25N3)

Vdrv_pri = 15        # Gate driver supply voltage (1EDN7511B)
Rhi_pri = 0.85       # High level (sourcing) output resistance (1EDN7511B)
Rlo_pri = 0.35       # Low level (sinking) output resistance (1EDN7511B)

Rgate_pri = 10       # External gate resistance

Vth_pri = 3          # Gate threshold voltage 
Vgsmiller_pri = 4.3  # Gate plateau voltage 

Ciss_pri = 1770e-12  # Input capacitance
Crss_pri = 3e-12     # Reverse transfer capacitance
Coss_pri = 112e-12   # Output capacitance

Rdson_pri = 51e-3    # Drain-source on-state resistance

Rgi_pri = 2.5        # Gate resistance

# SECONDARY MOSFET PARAMETERS  (IPA040N06NM5S)

Vdrv_sec = 15        # Gate driver supply voltage (1ED3120MC12H)
Rhi_sec = 0.95       # High level output on resistance (1ED3120MC12H)
Rlo_sec = 0.75       # Low level output on resistance (1ED3120MC12H)

Rgate_sec = 10       # External gate resistance

Vth_sec = 2.8        # Gate threshold voltage 
Vgsmiller_sec = 4.9  # Gate plateau voltage 

Ciss_sec = 2700e-12  # Input capacitance
Crss_sec = 28e-12    # Reverse transfer capacitance
Coss_sec = 670e-12   # Output capacitance

Rdson_sec = 3.6e-3   # Drain-source on-state resistance

Rgi_sec = 1.3        # Gate resistance

# H-BRIDGE MOSFET PARAMETERS

Rdson_hbridge = 2.9e-3

# OUTPUT CAPACITOR PARAMETERS

Vripple  = 120e-3    # 120 mV ripple target (2% of 6V)
ESR      = 0.1      # capacitor ESR


# CORE

cores = [

    {"name": "E5.3/2.7/2",   "Ae": 2.66e-6,  "Ve": 33.3e-9,   "le": 12.5e-3,  "Aw": 1.50e-6,   "MLT": 12.6e-3},
    {"name": "E6.3/2.9/2",   "Ae": 3.30e-6,  "Ve": 40.6e-9,   "le": 12.3e-3,  "Aw": 1.62e-6,   "MLT": 12.8e-3},
    {"name": "E13/6/6",      "Ae": 20.2e-6,  "Ve": 559e-9,    "le": 27.7e-3,  "Aw": 15.40e-6,  "MLT": 32.0e-3},
    {"name": "E13/7/4",      "Ae": 12.4e-6,  "Ve": 369e-9,    "le": 29.8e-3,  "Aw": 10.40e-6,  "MLT": 29.6e-3},
    {"name": "E16/7/5",      "Ae": 19.2e-6,  "Ve": 680e-9,    "le": 35.1e-3,  "Aw": 44.5e-6,   "MLT": 24.0e-3},
    {"name": "E16/8/5",      "Ae": 20.1e-6,  "Ve": 750e-9,    "le": 37.3e-3,  "Aw": 44.5e-6,   "MLT": 24.0e-3},
    {"name": "E19/8/5",      "Ae": 22.6e-6,  "Ve": 900e-9,    "le": 39.8e-3,  "Aw": 32.3e-6,   "MLT": 40.9e-3},
    {"name": "E19/8/9",      "Ae": 41.3e-6,  "Ve": 1650e-9,   "le": 40.0e-3,  "Aw": 39.7e-6,   "MLT": 45.2e-3},
    {"name": "E20/10/5",     "Ae": 31.2e-6,  "Ve": 1340e-9,   "le": 42.9e-3,  "Aw": 28.6e-6,   "MLT": 38.7e-3},
    {"name": "E20/10/6",     "Ae": 32.0e-6,  "Ve": 1490e-9,   "le": 46.0e-3,  "Aw": 34.0e-6,   "MLT": 41.2e-3},
    {"name": "E20/9/6",      "Ae": 32.1e-6,  "Ve": 1380e-9,   "le": 42.9e-3,  "Aw": 34.0e-6,   "MLT": 41.2e-3},
    {"name": "E25/10/6",     "Ae": 37.0e-6,  "Ve": 1930e-9,   "le": 52.2e-3,  "Aw": 47.4e-6,   "MLT": 53.1e-3},
    {"name": "E25/13/7",     "Ae": 52.0e-6,  "Ve": 2990e-9,   "le": 57.5e-3,  "Aw": 63.3e-6,   "MLT": 52.8e-3},

]


materials = [{"name": "N87", "k": 0.86,  "alpha": 1.5184, "beta": 2.7726}]


# ------------------------
# Converter power loss 
# ------------------------
#TI Appln note SLUA618A


# Primary MOSFET switching loss
def Ig2_on():
    return (Vdrv_pri - 0.5*(Vgsmiller_pri + Vth_pri)) / (Rhi_pri + Rgate_pri + Rgi_pri)

def Ig3_on():
    return (Vdrv_pri - Vgsmiller_pri) / (Rhi_pri + Rgate_pri + Rgi_pri)

def Ig3_off():
    return (0 - 0.5*(Vgsmiller_pri + Vth_pri)) / (Rlo_pri + Rgate_pri + Rgi_pri)

def Ig2_off():
    return (0 - Vgsmiller_pri) / (Rlo_pri + Rgate_pri + Rgi_pri)

def switching_times_on():
    ig2 = abs(Ig2_on())
    ig3 = abs(Ig3_on())
    t2 = Ciss_pri * (Vgsmiller_pri - Vth_pri) / ig2
    t3 = Crss_pri * Vdsoff / ig3
    return t2, t3
def switching_times_off():
    ig3 = abs(Ig3_off())
    ig2 = abs(Ig2_off())
    t3 = Ciss_pri * (Vgsmiller_pri - Vth_pri) / ig3
    t2 = Crss_pri * Vdsoff / ig2
    return t2, t3

def mosfet_switching_loss(Ipk, fsw):
    t2_on, t3_on = switching_times_on()
    t2_off, t3_off = switching_times_off()
    Ton  = t2_on + t3_on
    Toff = t2_off + t3_off
    T    = 1 / fsw
    P_on  = (Vdsoff * Ipk * Ton)  / (2 * T)
    P_off = (Vdsoff * Ipk * Toff) / (2 * T)
    return P_on + P_off

# Primary MOSFET conduction loss
def mosfet_conduction_loss(Irms):
    return Irms**2 * Rdson_pri

# Primary MOSFET output capacitor loss
def mosfet_coss_loss(fsw):
    return 0.5 * Coss_pri * Vdsoff**2 * fsw

# Primary MOSFET gate drive loss
Qg_pri = 22e-9;
def gate_drive_loss(fsw): 
    return Vdrv_pri * Qg_pri * fsw


# Secondary MOSFET switching loss

def Ig2_on_sec():
    return (Vdrv_sec - 0.5*(Vgsmiller_sec + Vth_sec)) / (Rhi_sec + Rgate_sec + Rgi_sec)

def Ig3_on_sec():
    return (Vdrv_sec - Vgsmiller_sec) / (Rhi_sec + Rgate_sec + Rgi_sec)

def Ig3_off_sec():
    return (0 - 0.5*(Vgsmiller_sec + Vth_sec)) / (Rlo_sec + Rgate_sec + Rgi_sec)

def Ig2_off_sec():
    return (0 - Vgsmiller_sec) / (Rlo_sec + Rgate_sec + Rgi_sec)

def switching_times_on_sec():
    ig2 = abs(Ig2_on_sec())
    ig3 = abs(Ig3_on_sec())
    t2 = Ciss_sec * (Vgsmiller_sec - Vth_sec) / ig2
    t3 = Crss_sec * Vdsoff_sec / ig3
    return t2, t3

def switching_times_off_sec():
    ig3 = abs(Ig3_off_sec())
    ig2 = abs(Ig2_off_sec())
    t3 = Ciss_sec * (Vgsmiller_sec - Vth_sec) / ig3
    t2 = Crss_sec * Vdsoff_sec / ig2
    return t2, t3

def mosfet_switching_loss_sec(Ipk_sec, fsw):
    t2_on, t3_on = switching_times_on_sec()
    t2_off, t3_off = switching_times_off_sec()
    Ton  = t2_on + t3_on
    Toff = t2_off + t3_off
    T    = 1 / fsw
    P_on  = (Vdsoff_sec * Ipk_sec * Ton)  / (2 * T)
    P_off = (Vdsoff_sec * Ipk_sec * Toff) / (2 * T)
    return P_on + P_off

# Secondary MOSFET conduction loss
def mosfet_conduction_loss_sec(Irms_sec):
    return Irms_sec**2 * Rdson_sec

# Secondary MOSFET output capacitor loss
def mosfet_coss_loss_sec(fsw):
    return 0.5 * Coss_sec * Vdsoff_sec**2 * fsw

# Secondary MOSFET gate drive loss
Qg_sec = 38e-9;
def gate_drive_loss_sec(fsw):
    return Vdrv_sec * Qg_sec * fsw

# Primary snubber loss
def snubber_loss(Ipk, Lm, fsw):
    Llk = k_leak * Lm
    return 0.5 * Llk * Ipk**2 * fsw * 1.72
    
# Secondary snubber loss
def snubber_loss_sec(Ipk_sec, Ls, fsw):
    Llks = k_leak * Ls
    return 0.5 * Llks * Ipk_sec**2 * fsw * 2

# H-bridge MOSFETs conduction loss
def mosfet_conduction_loss_hbridge():
    return 1**2 * Rdson_hbridge

def cu_loss_Dowell(N_layers, Rdc):
    
    A = 9.76e-5

    Fr = (
        A * (math.sinh(2 * A) + math.sin(2 * A)) /
        (math.cosh(2 * A) - math.cos(2 * A))
        +
        A**2 * (2 * (N_layers**2 - 1) / 3) *
        (math.sinh(A) - math.sin(A)) /
        (math.cosh(A) + math.cos(A))
    )
    
    R = Fr * Rdc
    return R

def core_loss_iGSE():

    alpha = 1.5184
    beta  = 2.7726

    # iGSE coefficient
    ki = 0.0399

    dBdt_on = dB_target / (4.5e-6)
    dBdt_off = dB_target / (5.5e-6)
    Pv = ki * (dB_target**(beta - alpha)) * (((0.43 * (dBdt_on)**alpha) + ((1 - 0.43) * (dBdt_off)**alpha)))
    print(f" Pv:      {Pv}\n")

    # Total core loss
    Pcore = Pv * 2 * core["Ve"]
    return Pcore


Lm_values  = np.linspace(50e-6, 500e-6, 50)
fsw_values = np.linspace(50e3, 200e3, 20)

results = []

# -------------------------
# MAIN LOOP
# -------------------------
for core in cores:
        for fsw in fsw_values:                        
            for Lm in Lm_values:

                
                Lm_crit =  (Vin**2 * D**2) / (2 * Pout * fsw)
                if Lm >= Lm_crit:
                    continue

                Ls = Lm / n**2

                Ipk   = (Vin * D) / (Lm * fsw)
                Iprms = Ipk * np.sqrt(D / 3)
                

                Ipk_sec  = Ipk * 6
                Isrms_sec  = Ipk_sec * np.sqrt((1 - D) / 3)
                
                P_deliverable = 0.5 * Lm * Ipk**2 * fsw
                if P_deliverable < Pout:
                    continue

                N = (Lm * Ipk) / (core["Ae"] * 0.1)
                if N < 2 or N > 100:
                    continue
                
                Ns = N/6

                lg = mu0 * N**2 * core["Ae"] / Lm - core["le"] / mur
                if lg <= 0 or lg > 3e-3:
                    continue

                Awire = Iprms / Jmax
                if N * Awire > Ku * core["Aw"]:
                    continue

                Rpri  = 1.72e-8 * core["MLT"] * N / Awire
                Racp = cu_loss_Dowell(1, Rpri)
                Pcu = Iprms**2 * Racp

                Rsec = 1.72e-8 * core["MLT"] * Ns / Awire
                Racs = cu_loss_Dowell(2, Rsec)
                Pcusec = Isrms_sec**2 * Racs

                Pcore = core_loss_iGSE()

                Ic_rmss = math.sqrt(((Ipk**2 * 6**2 * 5e-6 * fsw) / 3) - 1**2)
                Ic_rmsp = math.sqrt(((Ipk**2 * D) / 3) - (6 / 48 * 0.8)**2)

                P_cond   = mosfet_conduction_loss(Iprms)
                P_cond_sec = mosfet_conduction_loss_sec(Isrms_sec)

                P_sw     = mosfet_switching_loss(Ipk, fsw)
                P_sw_sec = mosfet_switching_loss_sec(Ipk_sec, fsw)

                P_coss   = mosfet_coss_loss(fsw)
                P_coss_sec = mosfet_coss_loss_sec(fsw)

                P_gate   = gate_drive_loss(fsw)
                P_gate_sec = gate_drive_loss_sec(fsw)

                P_snub   = snubber_loss(Ipk, Lm, fsw)
                P_snub_sec = snubber_loss_sec(Ipk_sec, Ls, fsw)

                P_cap = Ic_rmsp**2 * ESR
                P_cap_sec = Ic_rmss**2 * ESR
                                                          
                P_cond_hbridge = mosfet_conduction_loss_hbridge() 

                # real capacitor volume
                Vcap_m3 = 2 * 349e-9 #dia 6.3mm and length 11.2mm pi*r*r*h and mm3 to m3

                Ptotal = P_cond + P_cond_sec + P_sw + P_sw_sec + P_coss + P_coss_sec + P_gate + P_gate_sec + P_snub + P_snub_sec + Pcu + Pcusec + Pcore + P_cap_sec + P_cap + P_cond_hbridge

                if Ptotal >=3.0:
                    continue

                eta = Pout / (Pout + Ptotal)
                if eta < eta_min:
                    continue

                results.append({
                    "core":     core["name"],
                    "Lm":       Lm,
                    "fsw":      fsw,                 
                    "loss":     Ptotal,
                    "volume":   core["Ve"] + Vcap_m3 + Vcap_m3,
                })

# ----------------------------------------
# PARETO FRONT
# ----------------------------------------
def pareto_front(points):
    front = []
    for i, p in enumerate(points):
        dominated = False
        for j, q in enumerate(points):
            if i == j:
                continue
            if q["volume"] <= p["volume"] and q["loss"] <= p["loss"] \
               and (q["volume"] < p["volume"] or q["loss"] < p["loss"]):
                dominated = True
                break
        if not dominated:
            front.append(i)
    return front

front_idx = pareto_front(results)
front = sorted([results[i] for i in front_idx], key=lambda r: r["volume"])

print(f"Total feasible designs: {len(results)}")



# -------------------------
# PLOT
# -------------------------
plt.figure(figsize=(18, 12))

for core in cores:
    x = [r["volume"] * 1e9 for r in results if r["core"] == core["name"]]
    y = [r["loss"] for r in results if r["core"] == core["name"]]
    if x:
        plt.scatter(x, y, s=20, label=f'{core["name"]}')

xf = [r["volume"] * 1e9 for r in front]
yf = [r["loss"]   for r in front]
plt.plot(xf, yf, "k-o", linewidth=1, markersize=7,
         markerfacecolor="red", markeredgecolor="black", label="Pareto front", zorder=10)

for r in front:
    offset = 25
    plt.annotate(
        f"{r['core']}\n{r['Lm']*1e6:.0f}uH\n{r['fsw']/1e3:.0f}kHz",

        (r["volume"]*1e9, r["loss"]),

        fontsize=18,

        xytext=(offset, offset),
        textcoords="offset points",

        bbox=dict(
            facecolor="white",
            alpha=0.8,
            edgecolor="none"
        )
    )


plt.xlabel("Total Volume (mm^3)", fontsize=25)
plt.ylabel("Total Loss (W)", fontsize=25)
plt.title("Loss vs Volume", fontsize=25)
plt.grid(True)
plt.legend(fontsize=10, loc="upper right", ncol=2)
plt.xlim(1750, 4500)
plt.ylim(0.75, 1.8)
plt.tight_layout()
plt.xticks(fontsize=15) 
plt.yticks(fontsize=15)   
for r in results:

    if (
        r["core"] == "E16/7/5"
        and abs(r["Lm"] - 371e-6) < 5e-6
        and abs(r["fsw"] - 129e3) < 5e3
    ):
        
        x = r["volume"] * 1e9
        y = r["loss"]


        # highlighted point
        plt.scatter(
            x,
            y,
            s=100,                 
            color="red",
            edgecolors="black",
            linewidth=3,
            zorder=20
        )

        # label
        plt.annotate(
            "Selected Design",
            (x, y),
            fontsize=18,
            color="red",
            xytext=(-120, -80),
            textcoords="offset points",
            arrowprops=dict(
                arrowstyle="->",
                lw=3,
                color="red"
            )
        )

        break
plt.show()

