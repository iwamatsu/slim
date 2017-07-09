Quick THEME howto for SLiM

Some basic information regarding the slim theme format.
Read this file if you plan to make some theme for
the program, and of course have a look at the included themes

GENERAL CONCEPT
    A SLiM theme essentially consists of:
    - a background image (background.png or background.jpg)
    - a panel image (panel.png or panel.jpg)
    - input box(es) and messages and their placement
      and properties (slim.theme)
    
    The panel and background images can be a PNG or JPEG file.
    The panel is blended into the background image,
    taking care of alpha transparency.

SUPPORTED FORMATS
    - fonts: use the xft font specs, ie: Verdana:size=16:bold
    - colors: use html hex format, ie #0066CC
    - positions: can be either absolute in pixels, ie 350
      or relative to the container, ie 50% is in the middle
      of the screen.
    
OPTIONS
    The following is an example slim.theme file    
    ----------------------------------------------------------------------
    # Color, font, position for the messages (ie: shutting down)
    msg_color               #FFFFFF
    msg_font                Verdana:size=16:bold
    msg_x                   50%
    msg_y                   30

    # Color, font, position for the session list
    session_color           #FFFFFF
    session_font            Verdana:size=16:bold
    session_x               50%
    session_y               90%

    # style of background: 'stretch', 'tile', 'center', 'color'
    background_style        stretch
    background_color		#FF0033

    # Horizonatal and vertical position for the panel.
    input_panel_x           50%
    input_panel_y           40%
    
    # input controls horizontal and vertical positions.
    # IMPORTANT! set input_pass_x and input_pass_y to -1
    # to use a single input box for username/password (GDM Style).
    # Note that this fields only accept absolute values.
    input_name_x            40
    input_name_y            100
    input_pass_x            40
    input_pass_y            120
    
    # Input controls font and color
    input_font          	Verdana:size=12
    input_color             #000000

    # Welcome message position. (relative to the panel)
    # use -1 for both values or comment the options to disable
    # the welcome message
    welcome_x              	50%
    welcome_y              	38
    
    # Font and color for the welcome message
    welcome_font          	Verdana:size=16:bold:slant=italic
    welcome_color          	#d7dde8

    # 'Enter username' font and foreground/background color
    username_font          	Verdana:size=12
    username_color          	#d7dde8

    # 'Enter username' and 'Enter password' position (relative to the panel)
    #  use -1 for both values to disable the message
    # note that in case of single inputbox the password values are ignored.
    username_x                 50%
    username_y                 146
    password_x                 50%
    password_y                 146

    # The message to be displayed. Leave blank if no message
    # is needed (ie, when already present in the panel image)
    username_msg            Please enter your username
    password_msg            Please enter your password
    ----------------------------------------------------------------------

SHADOWS

    The 'msg', 'input', 'welcome', 'session' and 'username' sections
    support shadows; three values can be configured:
    - color: the shadow color
    - x offset: the offset in x direction, relative to the normal text 
    - y offset: the offset in y direction, relative to the normal text

    So to add a text shadow to the welcome message, add the following
    to slim.conf:
    ----------------------------------------------------------------------
    welcome_shadow_xoffset      -2
    welcome_shadow_yoffset      2
    welcome_shadow_color        #ff0000
    ----------------------------------------------------------------------
     
    The other keys are analogue:
    ----------------------------------------------------------------------
    # for username and password label
    username_shadow_xoffset 	2
    username_shadow_yoffset 	-2
    username_shadow_color   	#ff0000

    # for the input fields
    input_shadow_xoffset 	1
    input_shadow_yoffset 	1
    input_shadow_color   	#0000ff

    # for the messages:
    msg_shadow_xoffset 		1    
    msg_shadow_yoffset 		1    
    msg_shadow_color   		#ff00ff

    # For the session:
    session_shadow_xoffset 		1    
    session_shadow_yoffset 		1    
    session_shadow_color   		#ff00ff
    ----------------------------------------------------------------------

    
