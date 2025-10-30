#!/usr/bin/env python3
import ctypes
from ctypes import c_uint
import sys
import threading
import time
import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox

from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.ticker import MultipleLocator
import numpy as np

LIB_NAMES = ["dwf", "dwf.dll", "libdwf.so", "/usr/lib/libdwf.so", "/usr/local/lib/libdwf.so"]

# Ucitavanje DWF biblioteke
dwf = None
for name in LIB_NAMES:
    try:
        if sys.platform.startswith("win"):
            dwf = ctypes.windll.LoadLibrary(name)
        else:
            dwf = ctypes.cdll.LoadLibrary(name)
        break
    except Exception:
        dwf = None
if dwf is None:
    raise RuntimeError("Could not load DWF library. Install WaveForms SDK and/or update LIB_NAMES.")

# ---------- Common ctypes ----------
c_int = ctypes.c_int
c_double = ctypes.c_double
c_char_p = ctypes.c_char_p

# ---------- Function signatures (samo korištene) ----------
# Enumeration / device
dwf.FDwfEnum.restype = c_int
dwf.FDwfEnum.argtypes = [c_int, ctypes.POINTER(c_int)]
dwf.FDwfDeviceOpen.restype = c_int
dwf.FDwfDeviceOpen.argtypes = [c_int, ctypes.POINTER(c_int)]
dwf.FDwfDeviceClose.restype = c_int
dwf.FDwfDeviceClose.argtypes = [c_int]
dwf.FDwfDeviceParamSet = getattr(dwf, "FDwfDeviceParamSet", None)
if dwf.FDwfDeviceParamSet:
    dwf.FDwfDeviceParamSet.restype = c_int
    dwf.FDwfDeviceParamSet.argtypes = [c_int, c_int, c_int]

# ---- AnalogIO ----
FDwfAnalogIOChannelNodeSet = getattr(dwf, "FDwfAnalogIOChannelNodeSet", None)
if FDwfAnalogIOChannelNodeSet:
    FDwfAnalogIOChannelNodeSet.restype = c_int
    FDwfAnalogIOChannelNodeSet.argtypes = [c_int, c_int, c_int, c_double]
FDwfAnalogIOEnableSet = getattr(dwf, "FDwfAnalogIOEnableSet", None)
if FDwfAnalogIOEnableSet:
    FDwfAnalogIOEnableSet.restype = c_int
    FDwfAnalogIOEnableSet.argtypes = [c_int, c_int]
FDwfAnalogIOChannelEnableSet = getattr(dwf, "FDwfAnalogIOChannelEnableSet", None)
if FDwfAnalogIOChannelEnableSet:
    FDwfAnalogIOChannelEnableSet.restype = c_int
    FDwfAnalogIOChannelEnableSet.argtypes = [c_int, c_int, c_int]  # hdwf, idxChannel, fEnable
FDwfAnalogIOConfigure = getattr(dwf, "FDwfAnalogIOConfigure", None)
if FDwfAnalogIOConfigure:
    FDwfAnalogIOConfigure.restype = c_int
    FDwfAnalogIOConfigure.argtypes = [c_int] 

# ---- Analog Out (AWG)  ----
dwf.FDwfAnalogOutReset.restype = c_int
dwf.FDwfAnalogOutReset.argtypes = [c_int, c_int]  # hdwf, idxChannel
dwf.FDwfAnalogOutEnableSet.restype = c_int
dwf.FDwfAnalogOutEnableSet.argtypes = [c_int, c_int, c_int]
dwf.FDwfAnalogOutFunctionSet.restype = c_int
dwf.FDwfAnalogOutFunctionSet.argtypes = [c_int, c_int, c_int]
dwf.FDwfAnalogOutFrequencySet.restype = c_int
dwf.FDwfAnalogOutFrequencySet.argtypes = [c_int, c_int, c_double]
dwf.FDwfAnalogOutAmplitudeSet.restype = c_int
dwf.FDwfAnalogOutAmplitudeSet.argtypes = [c_int, c_int, c_double]
dwf.FDwfAnalogOutWaitSet.restype = c_int
dwf.FDwfAnalogOutWaitSet.argtypes = [c_int, c_int, c_double]
dwf.FDwfAnalogOutConfigure.restype = c_int
dwf.FDwfAnalogOutConfigure.argtypes = [c_int, c_int, c_int]
dwf.FDwfAnalogOutStatus.restype = c_int
dwf.FDwfAnalogOutStatus.argtypes = [c_int, c_int, ctypes.POINTER(c_int)]
dwf.FDwfAnalogOutOffsetSet = getattr(dwf, "FDwfAnalogOutOffsetSet", None)
if dwf.FDwfAnalogOutOffsetSet:
    dwf.FDwfAnalogOutOffsetSet.restype = c_int
    dwf.FDwfAnalogOutOffsetSet.argtypes = [c_int, c_int, c_double]

# ---- Analog In (Oscilloscope) ----
dwf.FDwfAnalogInReset.restype = c_int
dwf.FDwfAnalogInReset.argtypes = [c_int]
dwf.FDwfAnalogInConfigure.restype = c_int
dwf.FDwfAnalogInConfigure.argtypes = [c_int, c_int, c_int]  # hdwf, fReconfigure, fStart
dwf.FDwfAnalogInStatus.restype = c_int
dwf.FDwfAnalogInStatus.argtypes = [c_int, c_int, ctypes.POINTER(c_int)]  # hdwf, fReadData, *psts
dwf.FDwfAnalogInFrequencySet.restype = c_int
dwf.FDwfAnalogInFrequencySet.argtypes = [c_int, c_double]
dwf.FDwfAnalogInBufferSizeSet.restype = c_int
dwf.FDwfAnalogInBufferSizeSet.argtypes = [c_int, c_int]
dwf.FDwfAnalogInBufferSizeGet.restype = c_int
dwf.FDwfAnalogInBufferSizeGet.argtypes = [c_int, ctypes.POINTER(c_int)]
dwf.FDwfAnalogInAcquisitionModeSet.restype = c_int
dwf.FDwfAnalogInAcquisitionModeSet.argtypes = [c_int, c_int]  # ACQMODE
dwf.FDwfAnalogInTriggerSourceSet.restype = c_int
dwf.FDwfAnalogInTriggerSourceSet.argtypes = [c_int, c_int]  # TRIGSRC
dwf.FDwfAnalogInChannelEnableSet.restype = c_int
dwf.FDwfAnalogInChannelEnableSet.argtypes = [c_int, c_int, c_int]  # hdwf, idxCh, fEnable
dwf.FDwfAnalogInChannelRangeSet.restype = c_int
dwf.FDwfAnalogInChannelRangeSet.argtypes = [c_int, c_int, c_double]  # hdwf, idxCh, voltsRange
dwf.FDwfAnalogInStatusData.restype = c_int
dwf.FDwfAnalogInStatusData.argtypes = [c_int, c_int, ctypes.POINTER(c_double), c_int]  # hdwf, idxCh, *rg, cdData

# Digital IO
dwf.FDwfDigitalIOOutputEnableSet.restype = c_int
dwf.FDwfDigitalIOOutputEnableSet.argtypes = [c_int, c_int]
dwf.FDwfDigitalIOOutputSet.restype = c_int
dwf.FDwfDigitalIOOutputSet.argtypes = [c_int, c_int]
dwf.FDwfDigitalIOConfigure.restype = c_int
dwf.FDwfDigitalIOConfigure.argtypes = [c_int]
dwf.FDwfDigitalIOOutputEnableGet = getattr(dwf, "FDwfDigitalIOOutputEnableGet")
dwf.FDwfDigitalIOOutputEnableGet.restype = c_int
dwf.FDwfDigitalIOOutputEnableGet.argtypes = [c_int, ctypes.POINTER(c_uint)]
dwf.FDwfDigitalIOOutputGet = getattr(dwf, "FDwfDigitalIOOutputGet")
dwf.FDwfDigitalIOOutputGet.restype = c_int
dwf.FDwfDigitalIOOutputGet.argtypes = [c_int, ctypes.POINTER(c_uint)]

# Digital UART
dwf.FDwfDigitalUartReset.restype = c_int
dwf.FDwfDigitalUartReset.argtypes = [c_int]
dwf.FDwfDigitalUartRateSet.restype = c_int
dwf.FDwfDigitalUartRateSet.argtypes = [c_int, c_double]
dwf.FDwfDigitalUartBitsSet.restype = c_int
dwf.FDwfDigitalUartBitsSet.argtypes = [c_int, c_int]
dwf.FDwfDigitalUartParitySet.restype = c_int
dwf.FDwfDigitalUartParitySet.argtypes = [c_int, c_int]
dwf.FDwfDigitalUartStopSet.restype = c_int
dwf.FDwfDigitalUartStopSet.argtypes = [c_int, c_double]
dwf.FDwfDigitalUartPolaritySet.restype = c_int
dwf.FDwfDigitalUartPolaritySet.argtypes = [c_int, c_int]
dwf.FDwfDigitalUartTxSet.restype = c_int
dwf.FDwfDigitalUartTxSet.argtypes = [c_int, c_int]
dwf.FDwfDigitalUartRxSet.restype = c_int
dwf.FDwfDigitalUartRxSet.argtypes = [c_int, c_int]
dwf.FDwfDigitalUartTx.restype = c_int
dwf.FDwfDigitalUartTx.argtypes = [c_int, c_char_p, c_int]
dwf.FDwfDigitalUartRx.restype = c_int
dwf.FDwfDigitalUartRx.argtypes = [c_int, ctypes.c_void_p, c_int, ctypes.POINTER(c_int), ctypes.POINTER(c_int)]


# Pomoćne konstante (za SDK)
FUNC_SINE = 1
FUNC_SQUARE = 2
FUNC_TRIANGLE = 3
FUNC_DC = 7  
DwfParamDigitalVoltage = 13  # mV, when supported

# ---- AD3 Positive Supply @ 3.3 V ----
SUP_CH_POS = 0          # V+ is channel 0 on AD3
NODE_ENABLE = 0         # node 0 = enable
NODE_VOLTAGE = 1        # node 1 = voltage setpoint (0.5..5 V)

# AnalogIn constants
ACQMODE_SCAN_SCREEN = 2   # acqmodeScanScreen
TRIGSRC_NONE = 0          # trigsrcNone

# ---- State byte layout ----
BIT_I_CURRENT = 0      # b0: ima struje
BIT_GOING_DOWN = 1     # b1: rolete idu dolje
BIT_GOING_UP = 2       # b2: rolete idu gore
BIT_TOP = 3            # b3: gornja pozicija (otvoreno)
BIT_BOTTOM = 4         # b4: donja pozicija (zatvoreno)
BIT_SOFT = 5           # b5: soft start/stop
ADDR_SHIFT = 6         # b7..b6: adresa

# ---- 6-bit komande + 2-bit adresa u MSB ----
ADDR_2BIT = 0b01  # prva dva bita (MSB) = adresa 01

CMD_STOP = 0b111111     # ROLLER_STOP
CMD_UP   = 0b101010     # ROLLER_UP
CMD_DOWN = 0b010101     # ROLLER_DOWN
CMD_DBG  = 0b111000     # GET_DEBUG_INFO

def _build_frame(cmd6: int) -> int:
    """[addr(2)=01][cmd(6)] -> 8-bit bajt"""
    return ((ADDR_2BIT & 0b11) << 6) | (cmd6 & 0b111111)

# ---- Open device ----
hdwf = c_int(0)
ndev = c_int(0)
if not dwf.FDwfEnum(0, ctypes.byref(ndev)):
    raise RuntimeError("FDwfEnum failed")
if ndev.value < 1:
    raise RuntimeError("No DWF devices found")
if not dwf.FDwfDeviceOpen(-1, ctypes.byref(hdwf)):
    raise RuntimeError("FDwfDeviceOpen failed")

# Safe reset
dwf.FDwfAnalogOutReset(hdwf, -1)
dwf.FDwfDigitalUartReset(hdwf)

# ---- Try to set DIO voltage to 3.3 V  ----
def set_dio_voltage_3v3():
    if dwf.FDwfDeviceParamSet:
        try:
            dwf.FDwfDeviceParamSet(hdwf, DwfParamDigitalVoltage, 3300)
        except Exception:
            pass
    # AnalogIO VIO rail on ADP3450/3250 (no-op on others)
    if FDwfAnalogIOChannelNodeSet and FDwfAnalogIOEnableSet:
        try:
            # Channel 0, Node 0 = Digital Voltage (1.2–3.3 V)
            FDwfAnalogIOChannelNodeSet(hdwf, 0, 0, c_double(3.3))
            FDwfAnalogIOEnableSet(hdwf, 1)
        except Exception:
            pass

set_dio_voltage_3v3()

# Prikaz stanja sustava
class SystemStatePanel(ttk.LabelFrame):
    # Redoslijed prikaza zdesna nalijevo b5..b0 (svi horizontalno)
    ITEMS = [
        ("Soft start/stop", BIT_SOFT),
        ("Donja poz.",     BIT_BOTTOM),
        ("Gornja poz.",    BIT_TOP),
        ("Ide gore",       BIT_GOING_UP),
        ("Ide dolje",      BIT_GOING_DOWN),
        ("Struja",         BIT_I_CURRENT),
    ]

    def __init__(self, parent, title="Stanje (addr|b5..b0)", led_size=18):
        super().__init__(parent, text=title)
        self._led_size = led_size
        self._leds = []   # list[(canvas, oid)]
        self._labels = [] # list[StringVar] za tekst ispod lampice

        # Adresa (b7..b6)
        self.addr_var = tk.StringVar(value="Adresa: –")
        ttk.Label(self, textvariable=self.addr_var, font=("", 10, "bold"))\
            .grid(row=0, column=0, columnspan=len(self.ITEMS), sticky="w", padx=6, pady=(4,2))

        # Red 1: lampice (horizontalno)
        for col, (name, _bit) in enumerate(self.ITEMS):
            c = tk.Canvas(self, width=led_size+8, height=led_size+8, highlightthickness=0)
            c.grid(row=1, column=col, padx=6, pady=(2,0))
            oid = c.create_oval(4, 4, 4+led_size, 4+led_size, fill="#FF6B6B", outline="#111")
            self._leds.append((c, oid))

        # Red 2: natpisi ispod svake lampice
        for col, (name, _bit) in enumerate(self.ITEMS):
            v = tk.StringVar(value=name)
            ttk.Label(self, textvariable=v).grid(row=2, column=col, padx=6, pady=(0,6))
            self._labels.append(v)

        # Automatsko “prigušenje” ako nema updatea
        self._last_update_ts = time.time()
        self.after(300, self._idle_dim_check)

    def _set_led(self, idx, on: bool):
        c, oid = self._leds[idx]
        c.itemconfig(oid, fill="#21C55D" if on else "#FF6B6B")

    def update_from_state_byte(self, value: int):
        """value: 0..255; b7..b6=addr, b5..b0 status."""
        self._last_update_ts = time.time()
        v = int(value) & 0xFF
        addr = (v >> ADDR_SHIFT) & 0b11
        self.addr_var.set(f"Adresa (b7..b6): 0b{addr:02b}  (dec {addr})")

        # mapiraj šest lampica prema bitovima
        for idx, (_name, bit) in enumerate(self.ITEMS):
            self._set_led(idx, bool(v & (1 << bit)))

    def update_from_ascii_bits(self, bits: str):
        """Prihvaća string '01010101' i osvježava identično kao i bajt."""
        bits = ''.join(ch for ch in bits if ch in '01')[-8:].rjust(8, '0')
        v = int(bits, 2)
        self.update_from_state_byte(v)

    def _idle_dim_check(self):
        # if time.time() - self._last_update_ts > 1.0:
        #     for i in range(len(self.ITEMS)):
        #         self._set_led(i, False)
        # self.after(300, self._idle_dim_check)
        return  # disable for now


# GUI app
class WaveGenApp:
    def __init__(self, root):
        self.root = root
        root.title("WaveGen + UART + Scope (WaveForms SDK)")
        self.root.grid_columnconfigure(2, weight=1)
        self.root.grid_rowconfigure(0, weight=0)
        self.root.grid_rowconfigure(1, weight=0)
        self.root.grid_rowconfigure(2, weight=0)

        # Wavegen frames (left side)
        self.wg_frames = []
        for i in (0, 1):
            frm = ttk.LabelFrame(root, text=f"WaveGen {i+1}")
            frm.grid(row=0, column=i, padx=8, pady=6, sticky="n")
            self._build_wavegen_frame(frm, i)
            self.wg_frames.append(frm)

        # Global start with (software) delay (left)
        gfrm = ttk.Frame(root)
        gfrm.grid(row=1, column=0, columnspan=2, sticky="we", padx=8)
        ttk.Label(gfrm, text="Delay for WaveGen 2 (s):").grid(row=0, column=0, sticky="e")
        self.delay_var = tk.DoubleVar(value=2.0)
        ttk.Entry(gfrm, textvariable=self.delay_var, width=8).grid(row=0, column=1, sticky="w")
        ttk.Button(gfrm, text="Start Both (ch1 → delay → ch2)", command=self.start_both).grid(row=0, column=2, padx=10)

        # UART frame (left)
        uart = ttk.LabelFrame(root, text="UART (DIO0 TX, DIO1 RX)")
        uart.grid(row=2, column=0, columnspan=2, padx=8, pady=6, sticky="we")
        self._build_uart_frame(uart)

        # V+ 3.3V supply (left, below UART)
        sup = ttk.LabelFrame(root, text="V+ 3.3 V Supply")
        sup.grid(row=3, column=0, columnspan=2, padx=8, pady=6, sticky="we")
        self._build_supply33_frame(sup)

        # IO D2 set high for DE enable on RS485
        diofrm = ttk.LabelFrame(root, text="Digital I/O Control")
        diofrm.grid(row=4, column=0, columnspan=2, padx=8, pady=6, sticky="we")
        ttk.Button(diofrm, text="Set DIO2 HIGH", command=self.set_dio2_high).grid(row=0, column=0, padx=6, pady=6)
        ttk.Button(diofrm, text="Set DIO2 LOW", command=self.set_dio2_low).grid(row=0, column=1, padx=6, pady=6)

        # Scope (RIGHT side, spanning same vertical space)
        scope = ttk.LabelFrame(root, text="Oscilloscope (CH1 = 1±, CH2 = 2±)")
        scope.grid(row=0, column=2, rowspan=3, padx=8, pady=6, sticky="nswe")
        self._build_scope_frame(scope)

        # Polling / receive thread controls
        self._rx_running = threading.Event()
        self._rx_thread = None

        # Initialize defaults
        self.set_wave_defaults(0)
        self.set_wave_defaults(1)
        self.setup_uart_defaults()

    # -------- Wavegen UI --------
    def _build_wavegen_frame(self, parent, idx):
        led = tk.Canvas(parent, width=20, height=20)
        led.grid(row=0, column=0, padx=4, pady=4)
        led.circle = led.create_oval(2, 2, 18, 18, fill="red")
        setattr(self, f"led_{idx}", led)

        btn = ttk.Button(parent, text="Start", command=lambda i=idx: self.toggle_wave(i))
        btn.grid(row=0, column=1, padx=4)
        setattr(self, f"btn_{idx}", btn)

        ttk.Label(parent, text="Freq (Hz):").grid(row=1, column=0, sticky="e")
        fvar = tk.DoubleVar(value=50.0)
        ttk.Entry(parent, textvariable=fvar, width=8).grid(row=1, column=1, sticky="w")
        setattr(self, f"freq_{idx}", fvar)

        ttk.Label(parent, text="Amp (V):").grid(row=2, column=0, sticky="e")
        avar = tk.DoubleVar(value=5.0)
        ttk.Entry(parent, textvariable=avar, width=8).grid(row=2, column=1, sticky="w")
        setattr(self, f"amp_{idx}", avar)

        ttk.Label(parent, text="Type:").grid(row=3, column=0, sticky="e")
        type_combo = ttk.Combobox(parent, values=["sine", "square", "triangle", "dc"], state="readonly", width=8)
        type_combo.current(1)  # default square
        type_combo.grid(row=3, column=1)
        setattr(self, f"type_{idx}", type_combo)
   
    # ---- AD3 Positive Supply @ 3.3 V ----
    def _build_supply33_frame(self, parent):
        ttk.Button(parent, text="V+ 3.3 V ON", command=self.supply33_on)\
            .grid(row=0, column=0, padx=6, pady=6)
        ttk.Button(parent, text="V+ OFF", command=self.supply33_off)\
            .grid(row=0, column=1, padx=6, pady=6)

    def _apply_analogio(self):
        FDwfAnalogIOEnableSet(hdwf, 1)
        if FDwfAnalogIOConfigure:
            FDwfAnalogIOConfigure(hdwf)  

    def supply33_on(self):
        """Enable V+ and set it to +3.3 V on AD3."""
        try:
            # enable V+ channel via its Enable node, then set voltage
            FDwfAnalogIOChannelNodeSet(hdwf, SUP_CH_POS, NODE_ENABLE, c_double(1.0))
            FDwfAnalogIOChannelNodeSet(hdwf, SUP_CH_POS, NODE_VOLTAGE, c_double(3.3))
            self._apply_analogio()
        except Exception as e:
            messagebox.showerror("V+ 3.3 V", f"Failed to turn V+ ON:\n{e}")

    def supply33_off(self):
        """Disable only the V+ rail (leave V- untouched)."""
        try:
            FDwfAnalogIOChannelNodeSet(hdwf, SUP_CH_POS, NODE_ENABLE, c_double(0.0))
            self._apply_analogio()
        except Exception as e:
            messagebox.showerror("V+ OFF", f"Failed to turn V+ OFF:\n{e}")


    # -------- Scope (RIGHT, two plots) --------
    def _build_scope_frame(self, parent):
        # Controls row
        ctrl = ttk.Frame(parent)
        ctrl.grid(row=0, column=0, sticky="w")

        ttk.Button(ctrl, text="Start Scope", command=self.scope_start).grid(row=0, column=0, padx=4, pady=4)
        ttk.Button(ctrl, text="Stop Scope", command=self.scope_stop).grid(row=0, column=1, padx=4)

        ttk.Label(ctrl, text="Sample rate (Hz):").grid(row=0, column=2, sticky="e")
        self.scope_sr = tk.DoubleVar(value=100000)   # 100 kS/s
        ttk.Entry(ctrl, textvariable=self.scope_sr, width=10).grid(row=0, column=3, sticky="w", padx=2)

        ttk.Label(ctrl, text="Buffer:").grid(row=0, column=4, sticky="e")
        self.scope_buf = tk.IntVar(value=4096)
        ttk.Entry(ctrl, textvariable=self.scope_buf, width=8).grid(row=0, column=5, sticky="w", padx=2)

        # Channel enable and range
        ch = ttk.Frame(parent)
        ch.grid(row=1, column=0, sticky="w")
        self.ch1_en = tk.BooleanVar(value=True)
        self.ch2_en = tk.BooleanVar(value=True)
        ttk.Checkbutton(ch, text="Enable CH1", variable=self.ch1_en).grid(row=0, column=0, padx=4)
        ttk.Checkbutton(ch, text="Enable CH2", variable=self.ch2_en).grid(row=0, column=1, padx=4)

        ttk.Label(ch, text="CH1 Range (V):").grid(row=0, column=2, sticky="e")
        self.ch1_rng = tk.DoubleVar(value=5.0)
        ttk.Entry(ch, textvariable=self.ch1_rng, width=6).grid(row=0, column=3, sticky="w", padx=2)

        ttk.Label(ch, text="CH2 Range (V):").grid(row=0, column=4, sticky="e")
        self.ch2_rng = tk.DoubleVar(value=5.0)
        ttk.Entry(ch, textvariable=self.ch2_rng, width=6).grid(row=0, column=5, sticky="w", padx=2)

        # === ONE combined plot (CH1 + CH2) ===
        fig = Figure(figsize=(7.5, 3.5), dpi=100)
        ax = fig.add_subplot(111)

        ax.set_title("CH1 + CH2")
        ax.set_xlabel("Time (ms)")
        ax.set_ylabel("Voltage (V)")
        ax.grid(True)

        # Tick marks every 10 ms
        ax.xaxis.set_major_locator(MultipleLocator(10))

        self.ax = ax
        # CH1: solid blue, CH2: dashed red
        self.line1, = ax.plot([], [], color="blue", linestyle="-", label="CH1")
        self.line2, = ax.plot([], [], color="red", linestyle="--", label="CH2")
        ax.legend(loc="upper right")

        canvas = FigureCanvasTkAgg(fig, master=parent)
        canvas.get_tk_widget().grid(row=2, column=0, sticky="nsew", pady=4)
        self.scope_canvas = canvas
        self.scope_fig = fig
        parent.columnconfigure(0, weight=1)
        self._scope_run = threading.Event()
        self._scope_thread = None

    def scope_start(self):
        sr = float(self.scope_sr.get())
        nbuf = int(self.scope_buf.get())
        eff_buf = max(2 * nbuf, 16)  # double the time window (and keep a sane minimum)

        dwf.FDwfAnalogInReset(hdwf)
        dwf.FDwfAnalogInFrequencySet(hdwf, c_double(sr))
        dwf.FDwfAnalogInBufferSizeSet(hdwf, eff_buf)   # << use doubled buffer
        dwf.FDwfAnalogInAcquisitionModeSet(hdwf, ACQMODE_SCAN_SCREEN)
        dwf.FDwfAnalogInTriggerSourceSet(hdwf, TRIGSRC_NONE)

        # Start acquisition
        dwf.FDwfAnalogInConfigure(hdwf, 1, 1)  # reconfigure, start
        self._scope_run.set()
        if not self._scope_thread or not self._scope_thread.is_alive():
            self._scope_thread = threading.Thread(target=self._scope_loop, daemon=True)
            self._scope_thread.start()

    def scope_stop(self):
        self._scope_run.clear()
        # Stop acquisition gracefully
        try:
            dwf.FDwfAnalogInConfigure(hdwf, 0, 0)
        except Exception:
            pass

    def _scope_loop(self):
        # Pre-allocate buffers (we’ll resize if user changes buffer)
        buf_size = c_int(0)
        dwf.FDwfAnalogInBufferSizeGet(hdwf, ctypes.byref(buf_size))
        n = buf_size.value if buf_size.value > 0 else int(self.scope_buf.get())
        # numpy-backed buffers
        ch1_data = (c_double * n)()
        ch2_data = (c_double * n)()

        DwfState = c_int(0)
        while self._scope_run.is_set():
            dwf.FDwfAnalogInStatus(hdwf, 1, ctypes.byref(DwfState))  # 1 = read data
            # Pull channel data if enabled
            y1 = None; y2 = None
            if self.ch1_en.get():
                dwf.FDwfAnalogInStatusData(hdwf, 0, ch1_data, n)
                y1 = np.frombuffer(ch1_data, dtype=np.float64, count=n)
            if self.ch2_en.get():
                dwf.FDwfAnalogInStatusData(hdwf, 1, ch2_data, n)
                y2 = np.frombuffer(ch2_data, dtype=np.float64, count=n)

            # Update plots on the Tk thread
            def redraw():
                # current sample rate (Hz) -> x-axis in ms
                sr = float(self.scope_sr.get())
                if sr <= 0:
                    sr = 1.0
                # Plot CH1
                if y1 is not None:
                    t_ms = np.arange(y1.size) / sr * 1000.0
                    self.line1.set_data(t_ms, y1)
                else:
                    self.line1.set_data([], [])
                # Plot CH2
                if y2 is not None:
                    # reuse same time axis length as y2
                    t_ms2 = np.arange(y2.size) / sr * 1000.0
                    self.line2.set_data(t_ms2, y2)
                else:
                    self.line2.set_data([], [])
                # autoscale
                self.ax.relim()
                self.ax.autoscale_view()
                self.scope_canvas.draw_idle()

            try:
                self.root.after(0, redraw)
            except Exception:
                pass

            # modest refresh (~30–50 Hz)
            time.sleep(0.02)

    def _func_from_name(self, name):
        if name == "sine":
            return FUNC_SINE
        if name == "triangle":
            return FUNC_TRIANGLE
        if name == "dc":
            return FUNC_DC
        return FUNC_SQUARE

    def set_wave_defaults(self, idx):
        getattr(self, f"freq_{idx}").set(50.0)
        getattr(self, f"amp_{idx}").set(1.54)
        getattr(self, f"type_{idx}").set("dc")
        # ensure zero wait and enable
        dwf.FDwfAnalogOutWaitSet(hdwf, idx, c_double(0.0))
        dwf.FDwfAnalogOutEnableSet(hdwf, idx, 1)
        self.apply_wave_settings(idx)

    def apply_wave_settings(self, idx):
        func = self._func_from_name(getattr(self, f"type_{idx}").get())
        freq = float(getattr(self, f"freq_{idx}").get())
        amp  = float(getattr(self, f"amp_{idx}").get())

        dwf.FDwfAnalogOutFunctionSet(hdwf, idx, func)

        if func == FUNC_DC:
            # For DC, treat "Amp (V)" as the DC level (offset)
            if dwf.FDwfAnalogOutOffsetSet:
                dwf.FDwfAnalogOutOffsetSet(hdwf, idx, c_double(amp))
            # Make sure amplitude/frequency don't affect the output
            dwf.FDwfAnalogOutAmplitudeSet(hdwf, idx, c_double(0.0))
            dwf.FDwfAnalogOutFrequencySet(hdwf, idx, c_double(0.0))
        else:
            # Normal periodic wave
            dwf.FDwfAnalogOutFrequencySet(hdwf, idx, c_double(freq))
            dwf.FDwfAnalogOutAmplitudeSet(hdwf, idx, c_double(amp))

    def toggle_wave(self, idx):
        btn = getattr(self, f"btn_{idx}")
        if btn["text"] == "Start":
            self.apply_wave_settings(idx)
            dwf.FDwfAnalogOutWaitSet(hdwf, idx, c_double(0.0))
            dwf.FDwfAnalogOutConfigure(hdwf, idx, 1)
            btn.config(text="Stop")
            self._set_led(idx, True)
        else:
            dwf.FDwfAnalogOutConfigure(hdwf, idx, 0)
            btn.config(text="Start")
            self._set_led(idx, False)

    def _set_led(self, idx, on):
        led = getattr(self, f"led_{idx}")
        led.itemconfig(led.circle, fill="green" if on else "red")

    # IO D2 set high
    def set_dio2_high(self):
        try:
            # read current enabled outputs (avoid clobbering UART pins)
            enMask = c_uint(0)
            outMask = c_uint(0)
            dwf.FDwfDigitalIOOutputEnableGet(hdwf, ctypes.byref(enMask))
            dwf.FDwfDigitalIOOutputGet(hdwf, ctypes.byref(outMask))

            newEn = enMask.value | (1 << 2)
            newOut = outMask.value | (1 << 2)

            dwf.FDwfDigitalIOOutputEnableSet(hdwf, newEn)
            dwf.FDwfDigitalIOOutputSet(hdwf, newOut)
            dwf.FDwfDigitalIOConfigure(hdwf)  # apply


            messagebox.showinfo("DIO2", "DIO2 set HIGH (UART untouched)")
        except Exception as e:
            messagebox.showerror("DIO2", f"Failed to set DIO2 HIGH:\n{e}")

    # IO D2 set low
    def set_dio2_low(self):
        try:
            # read current enabled outputs (avoid clobbering UART pins)
            enMask = c_uint(0)
            outMask = c_uint(0)
            dwf.FDwfDigitalIOOutputEnableGet(hdwf, ctypes.byref(enMask))
            dwf.FDwfDigitalIOOutputGet(hdwf, ctypes.byref(outMask))

            bit = 1 << 2  # DIO2
            newEn = enMask.value | bit       # ensure DIO2 is enabled as output
            newOut = outMask.value & (~bit)  # clear DIO2 output bit (LOW)

            dwf.FDwfDigitalIOOutputEnableSet(hdwf, newEn)
            dwf.FDwfDigitalIOOutputSet(hdwf, newOut)
            dwf.FDwfDigitalIOConfigure(hdwf)  # apply

            messagebox.showinfo("DIO2", "DIO2 set LOW (UART untouched)")
        except Exception as e:
            messagebox.showerror("DIO2", f"Failed to set DIO2 LOW:\n{e}")

    # Simple software-delay startboth: ch0 now, sleep, ch1
    def start_both(self):
        self.apply_wave_settings(0)
        self.apply_wave_settings(1)
        delay = float(self.delay_var.get())

        # Start wavegen1 immediately
        dwf.FDwfAnalogOutWaitSet(hdwf, 0, c_double(0.0))
        dwf.FDwfAnalogOutConfigure(hdwf, 0, 1)
        self.btn_0.config(text="Stop")
        self._set_led(0, True)

        # Delay in background, then start wavegen2
        def start_second():
            time.sleep(max(0.0, delay))
            dwf.FDwfAnalogOutWaitSet(hdwf, 1, c_double(0.0))
            dwf.FDwfAnalogOutConfigure(hdwf, 1, 1)
            self.btn_1.config(text="Stop")
            self._set_led(1, True)
        threading.Thread(target=start_second, daemon=True).start()

    # -------- UART UI & logic --------
    def _build_uart_frame(self, parent):
        ttk.Label(parent, text="Baud:").grid(row=0, column=0, sticky="e")
        self.baud_var = tk.IntVar(value=400000)
        ttk.Entry(parent, textvariable=self.baud_var, width=8).grid(row=0, column=1, sticky="w")
        ttk.Button(parent, text="Apply UART Settings", command=self.apply_uart_settings).grid(row=0, column=2, padx=6)

        ttk.Label(parent, text="Send:").grid(row=1, column=0, sticky="e")
        self.tx_entry = tk.Entry(parent, width=40)
        self.tx_entry.grid(row=1, column=1, columnspan=2, sticky="w")
        ttk.Button(parent, text="Send (no CR only 8bits)", command=self.uart_send).grid(row=1, column=3, padx=6)

        self.recv_area = scrolledtext.ScrolledText(parent, width=70, height=10, state="disabled")
        self.recv_area.grid(row=2, column=0, columnspan=4, pady=6)

        self.rx_btn = ttk.Button(parent, text="Start Receiving", command=self.toggle_rx)
        self.rx_btn.grid(row=3, column=0, pady=4)

        ttk.Button(parent, text="Clear Screen", command=self.clear_uart_screen).grid(row=3, column=3, pady=4)

        ttk.Label(parent, text="RX buffer poll interval (s):").grid(row=3, column=1, sticky="e")
        self.poll_var = tk.DoubleVar(value=0.02)  # faster default polling
        ttk.Entry(parent, textvariable=self.poll_var, width=6).grid(row=3, column=2, sticky="w")

        # --- Motor state LEDs (left, under UART)
        self.state_panel = SystemStatePanel(root, title="Stanje (addr|b5..b0)")
        self.state_panel.grid(row=5, column=0, columnspan=2, padx=8, pady=(4,8), sticky="we")

        # quick command buttons
        btns = ttk.Frame(parent)
        btns.grid(row=4, column=0, columnspan=4, pady=(6, 0), sticky="w")

        ttk.Button(btns, text="STOP", width=16,
                command=lambda: self._send_cmd(CMD_STOP)).grid(row=0, column=0, padx=2)

        ttk.Button(btns, text="UP", width=16,
                command=lambda: self._send_cmd(CMD_UP)).grid(row=0, column=1, padx=2)

        ttk.Button(btns, text="DOWN", width=16,
                command=lambda: self._send_cmd(CMD_DOWN)).grid(row=0, column=2, padx=2)

        ttk.Button(btns, text="GET_STATES_INFO", width=16,
                command=lambda: self._send_cmd(CMD_DBG)).grid(row=0, column=3, padx=2)


    def clear_uart_screen(self):
        """Clear the UART receive text area."""
        self.recv_area.configure(state="normal")
        self.recv_area.delete("1.0", tk.END)
        self.recv_area.configure(state="disabled")

    def setup_uart_defaults(self):
        # Required fixed settings per your request:
        # polarity: standard(0), parity: none(0), stop: 1, bits: 8
        dwf.FDwfDigitalUartReset(hdwf)
        dwf.FDwfDigitalUartRateSet(hdwf, c_double(400000.0))
        dwf.FDwfDigitalUartBitsSet(hdwf, 8)
        dwf.FDwfDigitalUartParitySet(hdwf, 0)
        dwf.FDwfDigitalUartStopSet(hdwf, 1)
        dwf.FDwfDigitalUartPolaritySet(hdwf, 0)
        # Pins: default TX=DIO0, RX=DIO1 (change if your device maps differently)
        dwf.FDwfDigitalUartTxSet(hdwf, 0)
        dwf.FDwfDigitalUartRxSet(hdwf, 1)
        pcRx = c_int(0); pParity = c_int(0)
        dwf.FDwfDigitalUartRx(hdwf, None, 0, ctypes.byref(pcRx), ctypes.byref(pParity))

    def apply_uart_settings(self):
        baud = float(self.baud_var.get())
        dwf.FDwfDigitalUartRateSet(hdwf, c_double(baud))
        # Re-init RX after any setting change (just in case)
        pcRx = c_int(0); pParity = c_int(0)
        dwf.FDwfDigitalUartRx(hdwf, None, 0, ctypes.byref(pcRx), ctypes.byref(pParity))
        messagebox.showinfo("UART",
            f"UART {baud:.0f} baud (8N1). Slanje: točno 1 bajt bez CR.")

    def uart_send(self):
        s = self.tx_entry.get().strip().lower()
        if not s:
            return
        # Mapa naredbi
        mapping = {
            "stop": CMD_STOP,
            "up": CMD_UP,
            "down": CMD_DOWN,
            "dbg": CMD_DBG,
            "debug": CMD_DBG
        }
        if s in mapping:
            self._send_cmd(mapping[s])
            return
        # ili dopusti hex unos, npr. "0x45"
        try:
            val = int(s, 16) if s.startswith("0x") else int(s)
            val &= 0xFF
            buf = (ctypes.c_char * 1).from_buffer_copy(bytes([val]))
            dwf.FDwfDigitalUartTx(hdwf, buf, 1)
            self._append_recv(f"[TX BYTE] 0x{val:02X}\n")
        except Exception:
            messagebox.showerror("UART", "Upiši: stop | up | down | dbg | ili 1 bajt (npr. 0x45).")

    def _send_cmd(self, cmd6: int):
        """Pošalji jedan bajt [addr=01][cmd6]."""
        byte_val = _build_frame(cmd6)  # složi 8-bitni okvir
        buf = (ctypes.c_char * 1).from_buffer_copy(bytes([byte_val]))
        dwf.FDwfDigitalUartTx(hdwf, buf, 1)
        # Print command name as well as value in hex
        cmd_names = {
            CMD_STOP: "STOP",
            CMD_UP: "UP",
            CMD_DOWN: "DOWN",
            CMD_DBG: "GET_STATES_INFO"
        }
        cmd_name = cmd_names.get(cmd6, "UNKNOWN")
        self._append_recv(f"[TX - 0x{byte_val:02X}] -  {cmd_name}\n")

    def toggle_rx(self):
        if not self._rx_running.is_set():
            self._rx_running.set()
            self.rx_btn.config(text="Stop Receiving")
            self._rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
            self._rx_thread.start()
        else:
            self._rx_running.clear()
            self.rx_btn.config(text="Start Receiving")

    def _rx_loop(self):
        bufsize = 4096
        rxbuf = ctypes.create_string_buffer(bufsize)
        pcRx = c_int(0)
        pParity = c_int(0)

        bitbuf = bytearray()  # akumulira točno 8 ASCII bitova '0'/'1'

        while self._rx_running.is_set():
            # povuci što ima u RX baferu
            dwf.FDwfDigitalUartRx(hdwf, rxbuf, bufsize, ctypes.byref(pcRx), ctypes.byref(pParity))
            n = pcRx.value
            if n > 0:
                chunk = rxbuf.raw[:n]
                for b in chunk:
                    if b == ord('0') or b == ord('1'):
                        bitbuf.append(b)
                        if len(bitbuf) == 8:
                            # imamo puni okvir: 8 bitova kao ASCII
                            bits = bytes(bitbuf).decode('ascii')
                            v = int(bits, 2)
                            addr = (v >> ADDR_SHIFT) & 0b11

                            # log + ažuriraj GUI
                            self._append_recv(f"[RX STATE] 0x{v:02X}  addr={addr}  bits={bits}\n")
                            self.root.after(0, lambda vv=v: self.state_panel.update_from_state_byte(vv))

                            bitbuf.clear()
                    else:
                        # ignoriraj sve što nije '0' ili '1' (npr. CR/LF ili šum)
                        pass

            time.sleep(max(0.005, float(self.poll_var.get())))


    def _append_recv(self, text):
        self.recv_area.configure(state="normal")
        self.recv_area.insert(tk.END, text)
        self.recv_area.see(tk.END)
        self.recv_area.configure(state="disabled")

def on_close():
    try:
        app._rx_running.clear()
    except Exception:
        pass
    try:
        dwf.FDwfAnalogOutConfigure(hdwf, -1, 0)
    except Exception:
        pass
    try:
        dwf.FDwfDeviceClose(hdwf)
    except Exception:
        pass
    root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = WaveGenApp(root)
    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()
