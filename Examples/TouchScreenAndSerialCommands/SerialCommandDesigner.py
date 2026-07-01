import tkinter as tk
from tkinter import ttk, colorchooser, simpledialog, filedialog, messagebox
import serial
import serial.tools.list_ports
import time
import json
import os
import math

class ArduinoLCDController:
    def __init__(self, root):
        self.root = root
        self.root.title("Arduino LCD Controller")
        self.root.geometry("1200x700")
        
        # Screen dimensions
        self.screen_width = 480
        self.screen_height = 320
        
        # Serial connection
        self.serial_conn = None
        self.connected = False
        
        # Current drawing properties
        self.current_color = (255, 255, 255)  # Default: white
        self.current_text_size = 2
        self.current_tool = "select"
        self.selected_item = None
        
        # Store all drawn items
        self.canvas_items = []
        self.temp_item = None
        self.start_x = 0
        self.start_y = 0
        
        self.dragging = False
        self.resizing = False
        self.resize_handle = None
        self.last_x = 0
        self.last_y = 0
        
        # Add Delete key binding
        self.root.bind('<Delete>', self.delete_selected_item)
        
        # Bind directly to root window to handle movement globally
        self.root.bind('<Left>', self.handle_key_movement)
        self.root.bind('<Right>', self.handle_key_movement)
        self.root.bind('<Up>', self.handle_key_movement)
        self.root.bind('<Down>', self.handle_key_movement)
        self.root.bind('<Shift-Left>', self.handle_key_movement)
        self.root.bind('<Shift-Right>', self.handle_key_movement)
        self.root.bind('<Shift-Up>', self.handle_key_movement)
        self.root.bind('<Shift-Down>', self.handle_key_movement)
        
        self.setup_ui()
        self.update_listbox()  # Add this line
        
    def setup_ui(self):
        # Main frame
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Left panel for controls
        left_panel = ttk.LabelFrame(main_frame, text="Controls")
        left_panel.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)
        
        # Serial Connection
        conn_frame = ttk.LabelFrame(left_panel, text="Serial Connection")
        conn_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, padx=5, pady=5)
        
        self.port_combo = ttk.Combobox(conn_frame, width=15)
        self.port_combo.grid(row=0, column=1, padx=5, pady=5)
        self.refresh_ports()
        
        ttk.Button(conn_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=5, pady=5)
        ttk.Label(conn_frame, text="Baud:").grid(row=1, column=0, padx=5, pady=5)
        
        self.baud_combo = ttk.Combobox(conn_frame, width=15, values=["9600", "19200", "38400", "57600", "115200"])
        self.baud_combo.current(4)  # Default to 115200
        self.baud_combo.grid(row=1, column=1, padx=5, pady=5)
        
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=1, column=2, padx=5, pady=5)
        
        # Tools
        tools_frame = ttk.LabelFrame(left_panel, text="Tools")
        tools_frame.pack(fill=tk.X, padx=5, pady=5)
        
        tools = [
            ("select", "Select"),
            ("rect", "Rectangle"),
            ("fillrect", "Filled Rectangle"),
            ("roundrect", "Rounded Rectangle"),
            ("fillroundrect", "Filled Rounded Rect"),
            ("circle", "Circle"),
            ("fillcircle", "Filled Circle"),
            ("line", "Line"),
            ("text", "Text"),
            ("qrcode", "QR Code")
        ]
        
        row, col = 0, 0
        for tool_id, tool_name in tools:
            btn = ttk.Button(tools_frame, text=tool_name, width=15, 
                             command=lambda t=tool_id: self.select_tool(t))
            btn.grid(row=row, column=col, padx=3, pady=3)
            col += 1
            if col > 1:
                col = 0
                row += 1
				
        # Color selection
        color_frame = ttk.LabelFrame(left_panel, text="Color")
        color_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.color_preview = tk.Canvas(color_frame, width=30, height=30, bg="#FFFFFF")
        self.color_preview.grid(row=0, column=0, padx=5, pady=5)
        
        ttk.Button(color_frame, text="Choose Color", command=self.choose_color).grid(row=0, column=1, padx=5, pady=5)
        
        # Text size
        text_frame = ttk.LabelFrame(left_panel, text="Text Size")
        text_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.text_size_var = tk.IntVar(value=2)
        ttk.Label(text_frame, text="Size:").grid(row=0, column=0, padx=5, pady=5)
        
        size_spin = ttk.Spinbox(text_frame, from_=1, to=5, width=5, textvariable=self.text_size_var)
        size_spin.grid(row=0, column=1, padx=5, pady=5)
        
        # Right side container
        right_container = ttk.Frame(main_frame)
        right_container.pack(side=tk.RIGHT, fill=tk.Y, padx=5, pady=5)
        
        # Project actions panel on right side
        project_frame = ttk.LabelFrame(right_container, text="Project")
        project_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Button(project_frame, text="Clear Screen", command=self.clear_screen).pack(fill=tk.X, padx=5, pady=2)
        ttk.Button(project_frame, text="Save Design", command=self.save_design).pack(fill=tk.X, padx=5, pady=2)
        ttk.Button(project_frame, text="Load Design", command=self.load_design).pack(fill=tk.X, padx=5, pady=2)
        ttk.Button(project_frame, text="Send to Arduino", command=self.send_to_arduino).pack(fill=tk.X, padx=5, pady=2)
        ttk.Button(project_frame, text="Export Commands", command=self.export_commands).pack(fill=tk.X, padx=5, pady=2)
        
        # Center area for canvas and layers
        center_panel = ttk.Frame(main_frame)
        center_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Canvas frame
        canvas_frame = ttk.LabelFrame(center_panel, text=f"LCD Preview ({self.screen_width}x{self.screen_height})")
        canvas_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Canvas to represent LCD screen
        self.canvas = tk.Canvas(canvas_frame, width=self.screen_width, height=self.screen_height, bg="black")
        self.canvas.pack(padx=10, pady=10)
        
        # Canvas event bindings
        self.canvas.bind("<ButtonPress-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_canvas_release)

        # Layer Management below canvas
        layer_frame = ttk.LabelFrame(center_panel, text="Layers")
        layer_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Create listbox with scrollbar
        list_frame = ttk.Frame(layer_frame)
        list_frame.pack(fill=tk.BOTH, expand=True)
        
        # Add buttons frame on the right side of listbox
        list_controls = ttk.Frame(list_frame)
        list_controls.pack(side=tk.RIGHT, fill=tk.Y, padx=(5, 0))
        
        ttk.Button(list_controls, text="Edit Text", command=self.edit_text).pack(fill=tk.X, pady=2)
        ttk.Button(list_controls, text="Edit Color", command=self.edit_color).pack(fill=tk.X, pady=2)
        ttk.Button(list_controls, text="Text Size", command=self.edit_text_size).pack(fill=tk.X, pady=2)

        self.layer_listbox = tk.Listbox(list_frame, height=6, takefocus=0)
        self.layer_listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.layer_listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.layer_listbox.config(yscrollcommand=scrollbar.set)
        self.layer_listbox.bind('<<ListboxSelect>>', self.on_select_layer)
        
        # Layer movement buttons
        btn_frame = ttk.Frame(layer_frame)
        btn_frame.pack(fill=tk.X)
        
        ttk.Button(btn_frame, text="Move Up", command=self.move_layer_up).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="Move Down", command=self.move_layer_down).pack(side=tk.LEFT, padx=2)
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
        # Add Edit menu after creating main_frame
        menubar = tk.Menu(self.root)
        edit_menu = tk.Menu(menubar, tearoff=0)
        edit_menu.add_command(label="Delete", command=self.delete_selected_item)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        self.root.config(menu=menubar)
        
    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combo['values'] = ports
        if ports:
            self.port_combo.current(0)
            
    def toggle_connection(self):
        if not self.connected:
            port = self.port_combo.get()
            baud = int(self.baud_combo.get())
            
            try:
                self.serial_conn = serial.Serial(port, baud, timeout=1)
                time.sleep(2)  # Allow time for Arduino to reset
                self.connected = True
                self.connect_btn.config(text="Disconnect")
                self.status_var.set(f"Connected to {port} at {baud} baud")
            except Exception as e:
                messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")
                self.status_var.set("Connection failed")
        else:
            if self.serial_conn:
                self.serial_conn.close()
            self.connected = False
            self.connect_btn.config(text="Connect")
            self.status_var.set("Disconnected")
    
    def select_tool(self, tool):
        self.current_tool = tool
        self.status_var.set(f"Selected tool: {tool}")
    
    def choose_color(self):
        color = colorchooser.askcolor(title="Choose color")
        if color[1]:
            hex_color = color[1]
            self.color_preview.config(bg=hex_color)
            # Convert hex to RGB (0-255)
            r = int(hex_color[1:3], 16)
            g = int(hex_color[3:5], 16)
            b = int(hex_color[5:7], 16)
            self.current_color = (r, g, b)
    
    def clear_screen(self):
        self.canvas.delete("all")
        self.canvas_items = []
        self.selected_item = None
        self.layer_listbox.delete(0, tk.END)  # Clear the listbox
        if self.connected:
            self.send_command("clear|0|0|0")
    
    def rgb_to_hex(self, rgb):
        return f"#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}"
    
    def on_canvas_click(self, event):
        self.last_x = event.x
        self.last_y = event.y
        self.start_x = event.x  # Add this line
        self.start_y = event.y  # Add this line
        
        if self.current_tool == "select":
            # Check if clicking on resize handle
            if self.selected_item and self.is_over_resize_handle(event.x, event.y):
                self.resizing = True
                return
                
            # Check if clicking on an item
            self.selected_item = None
            for item in reversed(self.canvas_items):
                if self.is_point_in_item(event.x, event.y, item):
                    self.selected_item = item
                    self.dragging = True
                    self.highlight_selected_item()
                    return
                    
            self.canvas.delete("selection")
        
        elif self.current_tool == "text":
            text = simpledialog.askstring("Input", "Enter text:")
            if text:
                size = self.text_size_var.get()
                hex_color = self.rgb_to_hex(self.current_color)
                item_id = self.canvas.create_text(event.x, event.y, text=text, fill=hex_color, anchor="nw", 
                                                 font=("Arial", 10 * size))
                item = {
                    "type": "text",
                    "id": item_id,
                    "coords": [event.x, event.y],
                    "color": self.current_color,
                    "text": text,
                    "size": size
                }
                self.canvas_items.append(item)
                self.update_listbox()
        
        elif self.current_tool == "qrcode":
            data = simpledialog.askstring("Input", "Enter QR code data:", parent=self.root)
            if data:
                module_size = simpledialog.askinteger("Input", "Enter module size (pixels):", 
                                                    minvalue=1, maxvalue=10,
                                                    parent=self.root)
                if module_size:
                    size = module_size * 25
                    item_id = self.canvas.create_rectangle(
                        event.x, event.y, event.x + size, event.y + size,
                        fill=self.rgb_to_hex((0, 0, 0)), outline=self.rgb_to_hex((255, 255, 255))
                    )
                    item = {
                        "type": "qrcode",
                        "id": item_id,
                        "coords": [event.x, event.y],
                        "data": data,
                        "module_size": module_size,
                        "fg_color": (0, 0, 0),
                        "bg_color": (255, 255, 255)
                    }
                    self.canvas_items.append(item)
                    self.update_listbox()
        else:
            # For all other drawing tools, just set the start point
            # The actual drawing will happen in on_canvas_drag and on_canvas_release
            pass
    
    def on_canvas_drag(self, event):
        if self.current_tool == "select":
            if self.resizing and self.selected_item:
                self.resize_selected_item(event.x, event.y)
            elif self.dragging and self.selected_item:
                self.move_selected_item(event.x - self.last_x, event.y - self.last_y)
                
            self.last_x = event.x
            self.last_y = event.y
            return
        
        elif self.current_tool in ["rect", "fillrect", "roundrect", "fillroundrect"]:
            self.canvas.delete("temp")
            hex_color = self.rgb_to_hex(self.current_color)
            
            if self.current_tool in ["roundrect", "fillroundrect"]:
                if self.current_tool == "roundrect":
                    self.temp_item = self.canvas.create_rectangle(
                        self.start_x, self.start_y, event.x, event.y,
                        outline=hex_color, tags="temp"
                    )
                else:  # fillroundrect
                    self.temp_item = self.canvas.create_rectangle(
                        self.start_x, self.start_y, event.x, event.y,
                        fill=hex_color, outline=hex_color, tags="temp"
                    )
            else:
                if self.current_tool == "rect":
                    self.temp_item = self.canvas.create_rectangle(
                        self.start_x, self.start_y, event.x, event.y,
                        outline=hex_color, tags="temp"
                    )
                else:  # fillrect
                    self.temp_item = self.canvas.create_rectangle(
                        self.start_x, self.start_y, event.x, event.y,
                        fill=hex_color, outline=hex_color, tags="temp"
                    )
        
        elif self.current_tool in ["circle", "fillcircle"]:
            self.canvas.delete("temp")
            hex_color = self.rgb_to_hex(self.current_color)
            radius = ((event.x - self.start_x)**2 + (event.y - self.start_y)**2)**0.5
            
            if self.current_tool == "circle":
                self.temp_item = self.canvas.create_oval(
                    self.start_x - radius, self.start_y - radius,
                    self.start_x + radius, self.start_y + radius,
                    outline=hex_color, tags="temp"
                )
            else:  # fillcircle
                self.temp_item = self.canvas.create_oval(
                    self.start_x - radius, self.start_y - radius,
                    self.start_x + radius, self.start_y + radius,
                    fill=hex_color, outline=hex_color, tags="temp"
                )
        
        elif self.current_tool == "line":
            self.canvas.delete("temp")
            hex_color = self.rgb_to_hex(self.current_color)
            self.temp_item = self.canvas.create_line(
                self.start_x, self.start_y, event.x, event.y,
                fill=hex_color, tags="temp"
            )
    
    def on_canvas_release(self, event):
        self.dragging = False
        self.resizing = False
        
        if self.current_tool == "select":
            return
            
        self.canvas.delete("temp")
        hex_color = self.rgb_to_hex(self.current_color)
        
        if self.current_tool in ["rect", "fillrect"]:
            x1, y1 = self.start_x, self.start_y
            x2, y2 = event.x, event.y
            
            # Sort coordinates to ensure x1,y1 is the top-left and x2,y2 is the bottom-right
            if x1 > x2:
                x1, x2 = x2, x1
            if y1 > y2:
                y1, y2 = y2, y1
                
            if self.current_tool == "rect":
                item_id = self.canvas.create_rectangle(x1, y1, x2, y2, outline=hex_color)
            else:  # fillrect
                item_id = self.canvas.create_rectangle(x1, y1, x2, y2, fill=hex_color, outline=hex_color)
                
            self.canvas_items.append({
                "type": self.current_tool,
                "id": item_id,
                "coords": [x1, y1, x2, y2],
                "color": self.current_color
            })
            
        elif self.current_tool in ["roundrect", "fillroundrect"]:
            x1, y1 = self.start_x, self.start_y
            x2, y2 = event.x, event.y
            
            # Sort coordinates
            if x1 > x2:
                x1, x2 = x2, x1
            if y1 > y2:
                y1, y2 = y2, y1
                
            # Create initial rectangle
            if self.current_tool == "roundrect":
                item_id = self.canvas.create_rectangle(x1, y1, x2, y2, outline=hex_color)
            else:  # fillroundrect
                item_id = self.canvas.create_rectangle(x1, y1, x2, y2, fill=hex_color, outline=hex_color)
            
            # Store item temporarily
            item = {
                "type": self.current_tool,
                "id": item_id,
                "coords": [x1, y1, x2, y2],
                "color": self.current_color
            }
            self.canvas_items.append(item)
            
            # Now ask for radius
            radius = simpledialog.askinteger("Input", "Enter corner radius (pixels):", 
                                           minvalue=1, maxvalue=min((x2-x1)//2, (y2-y1)//2),
                                           initialvalue=20)
            if radius is not None:
                # Update with rounded corners
                if self.current_tool == "roundrect":
                    self.canvas.delete(item_id)
                    item_id = self.canvas.create_polygon(
                        self.create_rounded_rect_points(x1, y1, x2, y2, radius),
                        outline=hex_color, fill=""
                    )
                else:  # fillroundrect
                    self.canvas.delete(item_id)
                    item_id = self.canvas.create_polygon(
                        self.create_rounded_rect_points(x1, y1, x2, y2, radius),
                        fill=hex_color, outline=hex_color
                    )
                
                # Update stored item
                item["id"] = item_id
                item["radius"] = radius
            
        elif self.current_tool in ["circle", "fillcircle"]:
            radius = ((event.x - self.start_x)**2 + (event.y - self.start_y)**2)**0.5
            
            if self.current_tool == "circle":
                item_id = self.canvas.create_oval(
                    self.start_x - radius, self.start_y - radius,
                    self.start_x + radius, self.start_y + radius,
                    outline=hex_color
                )
            else:  # fillcircle
                item_id = self.canvas.create_oval(
                    self.start_x - radius, self.start_y - radius,
                    self.start_x + radius, self.start_y + radius,
                    fill=hex_color, outline=hex_color
                )
                
            self.canvas_items.append({
                "type": self.current_tool,
                "id": item_id,
                "coords": [self.start_x, self.start_y, int(radius)],
                "color": self.current_color
            })
            
        elif self.current_tool == "line":
            item_id = self.canvas.create_line(self.start_x, self.start_y, event.x, event.y, fill=hex_color)
            self.canvas_items.append({
                "type": "line",
                "id": item_id,
                "coords": [self.start_x, self.start_y, event.x, event.y],
                "color": self.current_color
            })
            
        self.update_listbox()
    
    def highlight_selected_item(self):
        self.canvas.delete("selection")
        if self.selected_item:
            item_type = self.selected_item["type"]
            if item_type in ["rect", "fillrect", "roundrect", "fillroundrect"]:
                x1, y1, x2, y2 = self.selected_item["coords"]
                # Draw selection rectangle
                self.canvas.create_rectangle(
                    x1-2, y1-2, x2+2, y2+2,
                    outline="#00FFFF", width=2, dash=(2, 4), tags="selection"
                )
                # Draw resize handle
                self.canvas.create_rectangle(
                    x2-5, y2-5, x2+5, y2+5,
                    fill="#00FFFF", tags="selection"
                )
                
            elif item_type in ["circle", "fillcircle"]:
                x, y, r = self.selected_item["coords"]
                # Draw selection circle
                self.canvas.create_oval(
                    x-r-2, y-r-2, x+r+2, y+r+2,
                    outline="#00FFFF", width=2, dash=(2, 4), tags="selection"
                )
                # Draw resize handle
                self.canvas.create_rectangle(
                    x+r-5, y-5, x+r+5, y+5,
                    fill="#00FFFF", tags="selection"
                )
            elif item_type == "line":
                x1, y1, x2, y2 = self.selected_item["coords"]
                self.canvas.create_line(
                    x1, y1, x2, y2,
                    fill="#00FFFF", width=3, dash=(2, 4), tags="selection"
                )
            elif item_type == "text":
                x, y = self.selected_item["coords"][:2]
                text = self.selected_item["text"]
                size = self.selected_item["size"]
                # Adjust text bounds calculation for better accuracy
                text_width = len(text) * 8 * size  # Increased from 6 to 8
                text_height = 12 * size  # Increased from 8 to 12
                self.canvas.create_rectangle(
                    x-2, y-2, x+text_width+2, y+text_height+2,
                    outline="#00FFFF", width=2, dash=(2, 4), tags="selection"
                )
            elif item_type == "qrcode":
                x, y = self.selected_item["coords"]
                size = self.selected_item["module_size"] * 25
                self.canvas.create_rectangle(
                    x-2, y-2, x+size+2, y+size+2,
                    outline="#00FFFF", width=2, dash=(2, 4), tags="selection"
                )
    
    def send_command(self, cmd):
        if self.connected and self.serial_conn:
            try:
                commands = cmd.split('\n')
                responses = []
                
                for command in commands:
                    if not command.strip():  # Skip empty commands
                        continue
                        
                    self.serial_conn.write((command + '\n').encode())
                    self.serial_conn.flush()  # Ensure command is sent
                    time.sleep(0.1)  # Increased delay between commands
                    
                    # Read response
                    response = self.serial_conn.readline().decode().strip()
                    if not response:  # If no response, try reading again
                        time.sleep(0.1)
                        response = self.serial_conn.readline().decode().strip()
                        
                    responses.append(response)
                    self.status_var.set(f"Sent: {command} - Response: {response}")
                    
                return responses[-1] if responses else None
            except Exception as e:
                messagebox.showerror("Serial Error", f"Failed to send command: {str(e)}")
                self.status_var.set(f"Error sending command: {str(e)}")
                return None
        else:
            self.status_var.set("Not connected to Arduino")
            return None
    
    def get_item_command(self, item):
        item_type = item["type"]
        
        if item_type == "qrcode":
            # QR codes use their own color settings
            x, y = item["coords"]
            module_size = item["module_size"]
            data = item["data"]
            fg_color = item.get("fg_color", (0, 0, 0))
            bg_color = item.get("bg_color", (255, 255, 255))
            commands = [f"drawQRCode|{data}|{x}|{y}|{module_size}|{bg_color[0]}|{bg_color[1]}|{bg_color[2]}|{fg_color[0]}|{fg_color[1]}|{fg_color[2]}"]
            commands.append("flush")
            return "\n".join(commands)
        
        # For all other items that use color
        color = item["color"]
        r, g, b = color
        
        # First set the color for the item
        commands = [f"setColor|{r}|{g}|{b}"]
        
        if item_type == "rect":
            x1, y1, x2, y2 = item["coords"]
            commands.append(f"drawRect|{x1}|{y1}|{x2-x1}|{y2-y1}")
        
        elif item_type == "fillrect":
            x1, y1, x2, y2 = item["coords"]
            commands.append(f"drawFillRect|{x1}|{y1}|{x2-x1}|{y2-y1}")
        
        elif item_type == "roundrect":
            x1, y1, x2, y2 = item["coords"]
            radius = item["radius"]
            commands.append(f"drawRoundRect|{x1}|{y1}|{x2-x1}|{y2-y1}|{radius}")
        
        elif item_type == "fillroundrect":
            x1, y1, x2, y2 = item["coords"]
            radius = item["radius"]
            commands.append(f"drawFillRoundRect|{x1}|{y1}|{x2-x1}|{y2-y1}|{radius}")
        
        elif item_type == "circle":
            x, y, radius = item["coords"]
            commands.append(f"drawCircleOutline|{x}|{y}|{radius}")
        
        elif item_type == "fillcircle":
            x, y, radius = item["coords"]
            commands.append(f"drawFillCircle|{x}|{y}|{radius}")
        
        elif item_type == "line":
            x1, y1, x2, y2 = item["coords"]
            commands.append(f"drawLine|{x1}|{y1}|{x2}|{y2}")
        
        elif item_type == "text":
            x, y = item["coords"]
            size = item["size"]
            text = item["text"]
            commands.append(f"prt|{text}|{x}|{y}|{size}")
        
        commands.append("flush")
        return "\n".join(commands)
    
    def save_design(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        save_data = []
        for item in self.canvas_items:
            # Create a copy of the item without the tkinter ID
            save_item = item.copy()
            save_item.pop('id', None)
            save_data.append(save_item)
        
        try:
            with open(file_path, 'w') as f:
                json.dump(save_data, f, indent=4)
            self.status_var.set(f"Design saved to {file_path}")
        except Exception as e:
            messagebox.showerror("Save Error", f"Failed to save design: {str(e)}")
            self.status_var.set(f"Error saving design: {str(e)}")
    
    def load_design(self):
        file_path = filedialog.askopenfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        try:
            with open(file_path, 'r') as f:
                load_data = json.load(f)
            
            # Clear current canvas
            self.canvas.delete("all")
            self.canvas_items = []
            
            for item in load_data:
                item_type = item["type"]
                color = tuple(item["color"])
                hex_color = self.rgb_to_hex(color)
                
                if item_type in ["rect", "fillrect"]:
                    x1, y1, x2, y2 = item["coords"]
                    if item_type == "rect":
                        item_id = self.canvas.create_rectangle(x1, y1, x2, y2, outline=hex_color)
                    else:  # fillrect
                        item_id = self.canvas.create_rectangle(x1, y1, x2, y2, fill=hex_color, outline=hex_color)
                
                elif item_type in ["roundrect", "fillroundrect"]:
                    x1, y1, x2, y2 = item["coords"]
                    radius = item.get("radius", 20)
                    if item_type == "roundrect":
                        item_id = self.canvas.create_polygon(
                            self.create_rounded_rect_points(x1, y1, x2, y2, radius),
                            outline=hex_color, fill=""
                        )
                    else:  # fillroundrect
                        item_id = self.canvas.create_polygon(
                            self.create_rounded_rect_points(x1, y1, x2, y2, radius),
                            fill=hex_color, outline=hex_color
                        )
                
                elif item_type in ["circle", "fillcircle"]:
                    x, y, r = item["coords"]
                    if item_type == "circle":
                        item_id = self.canvas.create_oval(x-r, y-r, x+r, y+r, outline=hex_color)
                    else:  # fillcircle
                        item_id = self.canvas.create_oval(x-r, y-r, x+r, y+r, fill=hex_color, outline=hex_color)
                
                elif item_type == "line":
                    x1, y1, x2, y2 = item["coords"]
                    item_id = self.canvas.create_line(x1, y1, x2, y2, fill=hex_color)
                
                elif item_type == "text":
                    x, y = item["coords"]
                    text = item["text"]
                    size = item["size"]
                    item_id = self.canvas.create_text(x, y, text=text, fill=hex_color, anchor="nw", 
                                                  font=("Arial", 10 * size))
                
                elif item_type == "qrcode":
                    x, y = item["coords"]
                    module_size = item["module_size"]
                    size = module_size * 25
                    item_id = self.canvas.create_rectangle(
                        x, y, x + size, y + size,
                        fill=self.rgb_to_hex((0, 0, 0)), outline=self.rgb_to_hex((255, 255, 255))
                    )
                
                # Add the item to canvas_items with the new ID
                new_item = item.copy()
                new_item["id"] = item_id
                self.canvas_items.append(new_item)
            
            self.status_var.set(f"Design loaded from {file_path}")
        except Exception as e:
            messagebox.showerror("Load Error", f"Failed to load design: {str(e)}")
            self.status_var.set(f"Error loading design: {str(e)}")
    
    def send_to_arduino(self):
        if not self.connected:
            messagebox.showerror("Connection Error", "Not connected to Arduino")
            return
        
        # First clear the screen
        self.send_command("clear|0|0|0")
        time.sleep(0.5)  # Give Arduino time to process
        
        # Send each item one by one
        total_items = len(self.canvas_items)
        for i, item in enumerate(self.canvas_items, 1):
            cmd = self.get_item_command(item)
            if cmd:
                response = self.send_command(cmd)
                self.status_var.set(f"Sending item {i}/{total_items}...")
                if not response:
                    messagebox.showerror("Communication Error", f"No response when sending item {i}")
                    break
                time.sleep(0.2)  # Longer delay between items
        
        self.status_var.set("Design sent to Arduino")
    
    def export_commands(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        try:
            with open(file_path, 'w') as f:
                for item in self.canvas_items:
                    cmd = self.get_item_command(item)
                    if cmd:
                        commands = cmd.split('\n')
                        for command in commands:
                            if not command.startswith('setColor') and not command == 'flush':
                                parts = command.split('|')
                                function = parts[0]
                                args = parts[1:]
                                
                                # Map command names to Arduino function names
                                function_map = {
                                    "drawRect": "screen.drawRect",
                                    "drawFillRect": "screen.fillRect",
                                    "drawRoundRect": "screen.drawRoundRect",
                                    "drawFillRoundRect": "screen.fillRoundRect",
                                    "drawCircleOutline": "screen.drawCircle",
                                    "drawFillCircle": "screen.fillCircle",
                                    "drawLine": "screen.drawLine",
                                    "prt": "screen.drawString",
                                    "drawQRCode": "screen.qrcode"
                                }
                                
                                arduino_function = function_map.get(function, function)
                                arduino_command = f"{arduino_function}({','.join(args)});"
                                f.write(f"{arduino_command}\n")
                
                self.status_var.set(f"Commands exported to {file_path}")
        except Exception as e:
            messagebox.showerror("Export Error", f"Failed to export commands: {str(e)}")
            self.status_var.set(f"Error exporting commands: {str(e)}")
    
    def is_point_in_item(self, x, y, item):
        item_type = item["type"]
        if item_type == "text":
            text_x, text_y = item["coords"]
            text_width = len(item["text"]) * 8 * item["size"]  # Match the new calculation
            text_height = 12 * item["size"]  # Match the new calculation
            return (text_x <= x <= text_x + text_width and 
                    text_y <= y <= text_y + text_height)
        elif item_type == "qrcode":
            qr_x, qr_y = item["coords"]
            size = item["module_size"] * 25
            return (qr_x <= x <= qr_x + size and 
                    qr_y <= y <= qr_y + size)
        elif item_type in ["rect", "fillrect", "roundrect", "fillroundrect"]:
            x1, y1, x2, y2 = item["coords"]
            return x1 <= x <= x2 and y1 <= y <= y2
        elif item_type in ["circle", "fillcircle"]:
            cx, cy, r = item["coords"]
            return (x - cx)**2 + (y - cy)**2 <= r**2
        elif item_type == "line":
            x1, y1, x2, y2 = item["coords"]
            # Simple line hit detection
            dist = abs((y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1) / ((y2-y1)**2 + (x2-x1)**2)**0.5
            return dist < 5
        # Add more hit detection for other shapes as needed
        return False

    def move_selected_item(self, dx, dy):
        if not self.selected_item:
            return
            
        item_type = self.selected_item["type"]
        
        if item_type == "text":
            x, y = self.selected_item["coords"]
            new_coords = [x + dx, y + dy]
            self.canvas.coords(self.selected_item["id"], *new_coords)
            self.selected_item["coords"] = new_coords
        
        elif item_type == "qrcode":
            x, y = self.selected_item["coords"]
            size = self.selected_item["module_size"] * 25
            new_coords = [x + dx, y + dy]
            self.canvas.coords(self.selected_item["id"], 
                             x + dx, y + dy, 
                             x + dx + size, y + dy + size)
            self.selected_item["coords"] = new_coords
        
        elif item_type in ["roundrect", "fillroundrect"]:
            x1, y1, x2, y2 = self.selected_item["coords"]
            radius = self.selected_item.get("radius", 20)
            new_coords = [x1 + dx, y1 + dy, x2 + dx, y2 + dy]
            
            # Redraw the rounded rectangle at new position
            hex_color = self.rgb_to_hex(self.selected_item["color"])
            self.canvas.delete(self.selected_item["id"])
            
            points = self.create_rounded_rect_points(*new_coords, radius)
            if item_type == "roundrect":
                self.selected_item["id"] = self.canvas.create_polygon(
                    points, outline=hex_color, fill="", smooth=True
                )
            else:  # fillroundrect
                self.selected_item["id"] = self.canvas.create_polygon(
                    points, fill=hex_color, outline=hex_color, smooth=True
                )
            
            self.selected_item["coords"] = new_coords
        
        # ... rest of existing move_selected_item code ...
        elif item_type in ["rect", "fillrect"]:
            x1, y1, x2, y2 = self.selected_item["coords"]
            new_coords = [x1 + dx, y1 + dy, x2 + dx, y2 + dy]
            self.canvas.coords(self.selected_item["id"], *new_coords)
            self.selected_item["coords"] = new_coords
            
        elif item_type in ["circle", "fillcircle"]:
            x, y, r = self.selected_item["coords"]
            new_coords = [x + dx, y + dy, r]
            self.canvas.coords(self.selected_item["id"], x + dx - r, y + dy - r, x + dx + r, y + dy + r)
            self.selected_item["coords"] = new_coords
            
        elif item_type == "line":
            x1, y1, x2, y2 = self.selected_item["coords"]
            new_coords = [x1 + dx, y1 + dy, x2 + dx, y2 + dy]
            self.canvas.coords(self.selected_item["id"], *new_coords)
            self.selected_item["coords"] = new_coords
            
        self.highlight_selected_item()
        self.update_listbox()

    def resize_selected_item(self, new_x, new_y):
        if not self.selected_item:
            return
            
        item_type = self.selected_item["type"]
        
        if item_type in ["roundrect", "fillroundrect"]:
            x1, y1, x2, y2 = self.selected_item["coords"]
            radius = self.selected_item.get("radius", 20)
            new_coords = [x1, y1, new_x, new_y]
            
            # Redraw with new size
            hex_color = self.rgb_to_hex(self.selected_item["color"])
            self.canvas.delete(self.selected_item["id"])
            
            points = self.create_rounded_rect_points(*new_coords, radius)
            if item_type == "roundrect":
                self.selected_item["id"] = self.canvas.create_polygon(
                    points, outline=hex_color, fill="", smooth=True
                )
            else:  # fillroundrect
                self.selected_item["id"] = self.canvas.create_polygon(
                    points, fill=hex_color, outline=hex_color, smooth=True
                )
            
            self.selected_item["coords"] = new_coords
        
        elif item_type in ["rect", "fillrect"]:
            x1, y1, x2, y2 = self.selected_item["coords"]
            new_coords = [x1, y1, new_x, new_y]
            self.canvas.coords(self.selected_item["id"], *new_coords)
            self.selected_item["coords"] = new_coords
            
        elif item_type in ["circle", "fillcircle"]:
            x, y, _ = self.selected_item["coords"]
            r = ((new_x - x)**2 + (new_y - y)**2)**0.5
            new_coords = [x, y, r]
            self.canvas.coords(self.selected_item["id"], x - r, y - r, x + r, y + r)
            self.selected_item["coords"] = new_coords
            
        self.highlight_selected_item()

    def is_over_resize_handle(self, x, y):
        if not self.selected_item:
            return False
            
        item_type = self.selected_item["type"]
        if item_type in ["rect", "fillrect", "roundrect", "fillroundrect"]:
            x2, y2 = self.selected_item["coords"][2:]
            return abs(x - x2) < 10 and abs(y - y2) < 10
        elif item_type in ["circle", "fillcircle"]:
            cx, cy, r = self.selected_item["coords"]
            handle_x = cx + r
            handle_y = cy
            return abs(x - handle_x) < 10 and abs(y - handle_y) < 10
        return False

    def delete_selected_item(self, event=None):
        if self.selected_item:
            self.canvas.delete(self.selected_item["id"])
            self.canvas.delete("selection")
            self.canvas_items.remove(self.selected_item)
            self.selected_item = None
            self.update_listbox()

    def move_selected_item_by_key(self, direction, step=1):
        if not self.selected_item:
            return
            
        dx = 0
        dy = 0
        
        if direction == 'left':
            dx = -step
        elif direction == 'right':
            dx = step
        elif direction == 'up':
            dy = -step
        elif direction == 'down':
            dy = step
            
        self.move_selected_item(dx, dy)

    def update_listbox(self):
        self.layer_listbox.delete(0, tk.END)
        for i, item in enumerate(self.canvas_items):
            desc = f"{i+1}. {item['type']}"
            
            # Add details based on item type
            if item['type'] == 'text':
                desc += f" - '{item['text']}'"
            elif item['type'] == 'qrcode':
                desc += f" - '{item['data']}'"
            
            # Add coordinates for all types
            coords_str = ', '.join(map(str, item['coords'][:2]))
            desc += f" at ({coords_str})"
            
            self.layer_listbox.insert(tk.END, desc)
            
            if item == self.selected_item:
                self.layer_listbox.selection_clear(0, tk.END)
                self.layer_listbox.selection_set(i)

    def on_select_layer(self, event):
        selection = self.layer_listbox.curselection()
        if selection:
            index = selection[0]
            self.selected_item = self.canvas_items[index]
            self.highlight_selected_item()

    def move_layer_up(self):
        selection = self.layer_listbox.curselection()
        if not selection or selection[0] == 0:
            return
            
        index = selection[0]
        self.canvas_items[index], self.canvas_items[index-1] = \
            self.canvas_items[index-1], self.canvas_items[index]
        self.redraw_canvas()
        self.update_listbox()
        self.layer_listbox.selection_clear(0, tk.END)
        self.layer_listbox.selection_set(index-1)

    def move_layer_down(self):
        selection = self.layer_listbox.curselection()
        if not selection or selection[0] == len(self.canvas_items) - 1:
            return
            
        index = selection[0]
        self.canvas_items[index], self.canvas_items[index+1] = \
            self.canvas_items[index+1], self.canvas_items[index]
        self.redraw_canvas()
        self.update_listbox()
        self.layer_listbox.selection_clear(0, tk.END)
        self.layer_listbox.selection_set(index+1)

    def redraw_canvas(self):
        self.canvas.delete("all")
        for item in self.canvas_items:
            self.redraw_item(item)
        if self.selected_item:
            self.highlight_selected_item()

    def redraw_item(self, item):
        hex_color = self.rgb_to_hex(item["color"]) if "color" in item else "#FFFFFF"
        
        if item["type"] in ["roundrect", "fillroundrect"]:
            x1, y1, x2, y2 = item["coords"]
            radius = item.get("radius", 20)
            points = self.create_rounded_rect_points(x1, y1, x2, y2, radius)
            
            if item["type"] == "roundrect":
                item["id"] = self.canvas.create_polygon(
                    points, outline=hex_color, fill="", smooth=True
                )
            else:  # fillroundrect
                item["id"] = self.canvas.create_polygon(
                    points, fill=hex_color, outline=hex_color, smooth=True
                )
            return
            
        # ... rest of existing redraw_item code ...
        elif item["type"] in ["rect", "fillrect"]:
            x1, y1, x2, y2 = item["coords"]
            if item["type"] == "rect":
                item["id"] = self.canvas.create_rectangle(x1, y1, x2, y2, outline=hex_color)
            else:  # fillrect
                item["id"] = self.canvas.create_rectangle(x1, y1, x2, y2, fill=hex_color, outline=hex_color)
        
        elif item["type"] in ["circle", "fillcircle"]:
            x, y, r = item["coords"]
            if item["type"] == "circle":
                item["id"] = self.canvas.create_oval(x-r, y-r, x+r, y+r, outline=hex_color)
            else:  # fillcircle
                item["id"] = self.canvas.create_oval(x-r, y-r, x+r, y+r, fill=hex_color, outline=hex_color)
        
        elif item["type"] == "line":
            x1, y1, x2, y2 = item["coords"]
            item["id"] = self.canvas.create_line(x1, y1, x2, y2, fill=hex_color)
        
        elif item["type"] == "text":
            x, y = item["coords"]
            text = item["text"]
            size = item["size"]
            item["id"] = self.canvas.create_text(x, y, text=text, fill=hex_color, anchor="nw",
                                               font=("Arial", 10 * size))
        
        elif item["type"] == "qrcode":
            x, y = item["coords"]
            module_size = item["module_size"]
            size = module_size * 25
            item["id"] = self.canvas.create_rectangle(
                x, y, x + size, y + size,
                fill=self.rgb_to_hex((0, 0, 0)), outline=self.rgb_to_hex((255, 255, 255))
            )

    def create_rounded_rect_points(self, x1, y1, x2, y2, radius):
        """Create point list for rounded rectangle with proper corner arcs"""
        # Ensure x1,y1 is top-left and x2,y2 is bottom-right
        if x1 > x2: x1, x2 = x2, x1
        if y1 > y2: y1, y2 = y2, y1
        
        # Limit radius to prevent overlap
        radius = min(radius, (x2-x1)//2, (y2-y1)//2)
        points = []
        
        # Add the points in sequence to form a complete shape
        points.extend([x1 + radius, y1])  # Top left start
        points.extend([x2 - radius, y1])  # Top edge
        
        # Top right corner
        for i in range(0, 91, 5):
            angle = i * math.pi / 180
            points.extend([
                x2 - radius + (radius * math.sin(angle)),
                y1 + radius - (radius * math.cos(angle))
            ])
        
        points.extend([x2, y1 + radius])  # Right edge start
        points.extend([x2, y2 - radius])  # Right edge end
        
        # Bottom right corner
        for i in range(0, 91, 5):
            angle = i * math.pi / 180
            points.extend([
                x2 - radius + (radius * math.cos(angle)),
                y2 - radius + (radius * math.sin(angle))
            ])
        
        points.extend([x2 - radius, y2])  # Bottom edge start
        points.extend([x1 + radius, y2])  # Bottom edge end
        
        # Bottom left corner
        for i in range(0, 91, 5):
            angle = i * math.pi / 180
            points.extend([
                x1 + radius - (radius * math.cos(angle)),
                y2 - radius + (radius * math.sin(angle))
            ])
        
        points.extend([x1, y2 - radius])  # Left edge start
        points.extend([x1, y1 + radius])  # Left edge end
        
        # Top left corner
        for i in range(0, 91, 5):
            angle = i * math.pi / 180
            points.extend([
                x1 + radius - (radius * math.sin(angle)),
                y1 + radius - (radius * math.cos(angle))
            ])
        
        points.extend([x1 + radius, y1])  # Close the shape
        return points

    def handle_key_movement(self, event):
        """Handle arrow key movement, preventing listbox from processing keys"""
        if event.state & 0x1:  # Check if Shift is pressed
            step = 10
        else:
            step = 1
            
        if event.keysym == 'Left':
            self.move_selected_item_by_key('left', step)
        elif event.keysym == 'Right':
            self.move_selected_item_by_key('right', step)
        elif event.keysym == 'Up':
            self.move_selected_item_by_key('up', step)
        elif event.keysym == 'Down':
            self.move_selected_item_by_key('down', step)
        
        return "break"  # Prevent event from propagating

    def edit_text(self):
        if not self.selected_item or self.selected_item["type"] != "text":
            return
            
        new_text = simpledialog.askstring("Edit Text", "Enter new text:", 
                                        initialvalue=self.selected_item["text"],
                                        parent=self.root)
        if new_text:
            self.selected_item["text"] = new_text
            self.canvas.itemconfig(self.selected_item["id"], text=new_text)
            self.update_listbox()
    
    def edit_color(self):
        if not self.selected_item:
            return
            
        current_color = self.rgb_to_hex(self.selected_item["color"])
        color = colorchooser.askcolor(title="Choose color", 
                                    color=current_color,
                                    parent=self.root)
        if color[1]:
            hex_color = color[1]
            r = int(hex_color[1:3], 16)
            g = int(hex_color[3:5], 16)
            b = int(hex_color[5:7], 16)
            self.selected_item["color"] = (r, g, b)
            
            # Update the item's color
            item_type = self.selected_item["type"]
            if item_type == "text":
                self.canvas.itemconfig(self.selected_item["id"], fill=hex_color)
            elif item_type in ["rect", "roundrect", "line"]:
                self.canvas.itemconfig(self.selected_item["id"], outline=hex_color)
            elif item_type in ["fillrect", "fillroundrect", "fillcircle"]:
                self.canvas.itemconfig(self.selected_item["id"], fill=hex_color, outline=hex_color)
            elif item_type == "circle":
                self.canvas.itemconfig(self.selected_item["id"], outline=hex_color)
            
            self.update_listbox()

    def edit_text_size(self):
        if not self.selected_item or self.selected_item["type"] != "text":
            return
            
        new_size = simpledialog.askinteger("Edit Text Size", "Enter text size (1-5):", 
                                          initialvalue=self.selected_item["size"],
                                          minvalue=1, maxvalue=5,
                                          parent=self.root)
        if new_size:
            self.selected_item["size"] = new_size
            self.canvas.itemconfig(
                self.selected_item["id"], 
                font=("Arial", 10 * new_size)
            )
            self.update_listbox()

if __name__ == "__main__":
    root = tk.Tk()
    app = ArduinoLCDController(root)
    root.mainloop()