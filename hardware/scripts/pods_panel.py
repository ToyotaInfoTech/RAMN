# from https://gist.github.com/maxanier/87a9e73fbd655bf040c3cead2ff43e65 MIT license
# changes copyright TMNA 2024

from kikit import panelize_ui_impl as ki
from kikit.units import mm, deg
from kikit.panelize import Panel, BasicGridPosition, Origin
from pcbnewTransition.pcbnew import LoadBoard, VECTOR2I
from pcbnewTransition import pcbnew
from shapely.geometry import box
from itertools import chain


############### Custom config
board0_path = "./V1_revB/0_ramn/ramn.kicad_pcb"
board1_path = "./V1_revB/1_screens/screens.kicad_pcb"
board2_path = "./V1_revB/2_chassis/chassis.kicad_pcb"
board3_path = "./V1_revB/3_powertrain/powertrain.kicad_pcb"
board4_path = "./V1_revB/4_body/body.kicad_pcb"
board5_path = "./V1_revB/5_debugger/debugger.kicad_pcb"  # not used in this panel

output_path = "build/pods_panel.kicad_pcb"


################ KiKit Panel Config (Only deviations from default)
board_spacing = 3 * mm
framing = { "type": "railstb",
           "width": "5mm",
           "space": str(int(board_spacing / mm)) + "mm",
           "fillet": "1mm"
           }

cuts = {"type": "mousebites", "drill": "0.5mm", "spacing": "0.9mm"}
tabs = {  # Add tabs between board and board as well as board and rail
    "type": "fixed",  # Place them with constant width and spacing
    "width": "6mm",
    "vcount": "1",
    "hcount": "1"
}

tooling = {"type": "3hole", "hoffset": "2.5mm", "voffset": "2.5mm", "size": "1.5mm"}
fiducials = {"type": "3fid", "hoffset": "5mm", "voffset": "2.5mm", "coppersize": "2mm", "opening": "1mm"}

# Obtain full config by combining above with default
preset = ki.obtainPreset([], tabs=tabs, cuts=cuts, framing=framing, tooling=tooling, fiducials=fiducials)


################ Adjusted `panelize_ui#doPanelization`

# Prepare
board0 = LoadBoard(board0_path)
board1 = LoadBoard(board1_path)
board2 = LoadBoard(board2_path)
board3 = LoadBoard(board3_path)
board4 = LoadBoard(board4_path)
board5 = LoadBoard(board5_path)

panel = Panel(output_path)
panel.inheritDesignSettings(board0)
panel.inheritProperties(board0)
panel.inheritTitleBlock(board0)

###### Manually build layout. Inspired by `panelize_ui_impl#buildLayout`
sourceArea1 = ki.readSourceArea(preset["source"], board1)
sourceArea2 = ki.readSourceArea(preset["source"], board2)
sourceArea3 = ki.readSourceArea(preset["source"], board3)
sourceArea4 = ki.readSourceArea(preset["source"], board4)
sourceArea5 = ki.readSourceArea(preset["source"], board5)

substrateCount = len(panel.substrates)  # Store number of previous boards (probably 0)
# Prepare renaming nets and references
netRenamer = lambda x, y: "Board_{n}-{orig}".format(n=x, orig=y)
refRenamer = lambda x, y: "{orig}".format(n=x, orig=y)

# Actually place the individual boards
# Use existing grid positioner
# Place two boards above each other
panelOrigin = VECTOR2I(0, 0)
placer = BasicGridPosition(board_spacing, board_spacing)  # HorSpace, VerSpace
bodypod_area = panel.appendBoard(
    board4_path,
    panelOrigin + placer.position(0, 0, None),
    origin=Origin.Center,
    sourceArea=sourceArea4,
    netRenamer=netRenamer,
    refRenamer=refRenamer,
    inheritDrc=False,
)
screenpod_area = panel.appendBoard(
    board1_path,
    panelOrigin + placer.position(0, -2, bodypod_area),
    origin=Origin.Center,
    rotationAngle=ki.fromDegrees(90),
    sourceArea=sourceArea1,
    netRenamer=netRenamer,
    refRenamer=refRenamer,
    inheritDrc=False,
)
powertrainpod_area = panel.appendBoard(
    board3_path,
    panelOrigin + placer.position(0, -3, bodypod_area),
    origin=Origin.Center,
    rotationAngle=ki.fromDegrees(90),
    sourceArea=sourceArea3,
    netRenamer=netRenamer,
    refRenamer=refRenamer,
    inheritDrc=False,
)
chassispod_area = panel.appendBoard(
    board2_path,
    panelOrigin + placer.position(0, -1, bodypod_area),
    origin=Origin.Center,
    rotationAngle=ki.fromDegrees(0),
    sourceArea=sourceArea2,
    netRenamer=netRenamer,
    refRenamer=refRenamer,
    inheritDrc=False,
)

substrates = panel.substrates[substrateCount:]  # Collect set of newly added boards

# Prepare frame and partition
framingSubstrates = ki.dummyFramingSubstrate(substrates, preset)
panel.buildPartitionLineFromBB(framingSubstrates, safeMargin=4*mm)
#backboneCuts = ki.buildBackBone(preset["layout"], panel, substrates, preset)
backboneCuts = []


######## --------------------- Continue doPanelization
tabCuts = ki.buildTabs(preset, panel, substrates, framingSubstrates)

frameCuts = ki.buildFraming(preset, panel)

# the pods have very different overall dimensions; add extra substrate inside the rail for the smaller pods
def add_rail_bumps(target_area, panel):
    #NB: minx, miny, maxx, maxy = substrates[0].bounds()
    bottom_bump = box(target_area.GetLeft(),
            target_area.GetBottom() + board_spacing,
            target_area.GetRight(),
            framingSubstrates[1].bounds()[3])  # 3 is maxy
    panel.appendSubstrate(bottom_bump)

    top_bump = box(target_area.GetLeft(),
            target_area.GetTop() - board_spacing,
            target_area.GetRight(),
            framingSubstrates[0].bounds()[1])  # 1 is miny
    panel.appendSubstrate(top_bump)
    return

add_rail_bumps(screenpod_area, panel)
add_rail_bumps(powertrainpod_area, panel)
add_rail_bumps(chassispod_area, panel)

ki.buildTooling(preset, panel)
ki.buildFiducials(preset, panel)
for textSection in ["text", "text2", "text3", "text4"]:
    ki.buildText(preset[textSection], panel)
ki.buildPostprocessing(preset["post"], panel)

ki.makeTabCuts(preset, panel, tabCuts)
ki.makeOtherCuts(preset, panel, chain(backboneCuts, frameCuts))


ki.buildCopperfill(preset["copperfill"], panel)

ki.setStackup(preset["source"], panel)
ki.setPageSize(preset["page"], panel, board0)
ki.positionPanel(preset["page"], panel)

ki.runUserScript(preset["post"], panel)

ki.buildDebugAnnotation(preset["debug"], panel)

panel.save(
    reconstructArcs=preset["post"]["reconstructarcs"],
    refillAllZones=preset["post"]["refillzones"],
)
