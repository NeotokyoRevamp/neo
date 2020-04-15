"resource/neo_ui/Neo_TeamMenu.res"
{
	"team"
	{
		"ControlName"		"CNeoTeamMenu"
		"fieldName"		"team"
		"xpos"		"343"
		"ypos"		"228"
		"wide"		"360"
		"tall"		"215"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"settitlebarvisible"		"0"
		"title"		"TEAM"
	}
	"jinraibutton"
	{
		"ControlName"		"Button"
		"fieldName"		"jinraibutton"
		"xpos"		"10"
		"ypos"		"130"
		"wide"		"110"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"3"
		"labelText"		"JINRAI  "
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"font"		"DefaultBold"
		"wrap"		"0"
		"Command"		"jointeam 2"
		"Default"		"0"
	}
	"ctbutton"
	{
		"ControlName"		"Button"
		"fieldName"		"ctbutton"
		"xpos"		"125"
		"ypos"		"130"
		"wide"		"110"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"4"
		"labelText"		"NSF  "
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"font"		"DefaultBold"
		"wrap"		"0"
		"Command"		"jointeam 3"
		"Default"		"0"
	}
	"autobutton"
	{
		"ControlName"		"Button"
		"fieldName"		"autobutton"
		"xpos"		"240"
		"ypos"		"81"
		"wide"		"110"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"1"
		"labelText"		"AUTO ASSIGN   "
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"font"		"DefaultBold"
		"wrap"		"0"
		"Command"		"jointeam 0"
		"Default"		"0"
	}
	"specbutton"
	{
		"ControlName"		"Button"
		"fieldName"		"specbutton"
		"xpos"		"240"
		"ypos"		"106"
		"wide"		"110"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"2"
		"labelText"		"SPECTATE  "
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"font"		"DefaultBold"
		"wrap"		"0"
		"Command"		"jointeam 1"
		"Default"		"0"
	}
	"CancelButton"
	{
		"ControlName"		"Button"
		"fieldName"		"CancelButton"
		"xpos"		"240"
		"ypos"		"130"
		"wide"		"110"
		"tall"		"20"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"CANCEL   "
		"textAlignment"		"center"
		"dulltext"		"0"
		"brighttext"		"0"
		"font"		"DefaultBold"
		"wrap"		"0"
		"Command"		"vguicancel"
		"Default"		"0"
	}
	"ImagePanel1"
	{
		"ControlName"		"ImagePanel"
		"fieldName"		"ImagePanel1"
		"xpos"		"10"
		"ypos"		"21"
		"wide"		"110"
		"tall"		"110"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"image"		"ts_jinrai"
		"scaleImage"		"1"
		"zpos"	"1000"
	}
	"ImagePanel2"
	{
		"ControlName"		"ImagePanel"
		"fieldName"		"ImagePanel2"
		"xpos"		"125"
		"ypos"		"21"
		"wide"		"110"
		"tall"		"110"
		"autoResize"		"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"image"		"ts_nsf"
		"scaleImage"		"1"
	}
}
