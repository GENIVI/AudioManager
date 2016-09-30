var NAVTREE =
[
  [ "AudioManager", "index.html", [
    [ "License", "lic.html", [
      [ "Open Source Projects in the source tree", "lic.html#proj", null ],
      [ "License Split", "lic.html#split", null ],
      [ "Mozilla Public License, v. 2.0", "lic.html#mpl", null ],
      [ "MIT license", "lic.html#mit", null ]
    ] ],
    [ "Dependencies", "dep.html", [
      [ "Dependency Graph", "dep.html#deps", null ],
      [ "Depedency Graph for Tests", "dep.html#deptest", null ],
      [ "Generated Dependency Graph", "dep.html#depgen", null ]
    ] ],
    [ "Versioning", "ver.html", [
      [ "New versioning scheme", "ver.html#new_ver", null ],
      [ "The versioning scheme until 7.0", "ver.html#ver_graph", null ]
    ] ],
    [ "Architecture Overview", "architecturepage.html", [
      [ "Audio Domains", "architecturepage.html#domains", null ],
      [ "Routing Adapter", "architecturepage.html#routing_adaptor", null ],
      [ "Gateway", "architecturepage.html#gateway", null ],
      [ "Converter", "architecturepage.html#converter", null ]
    ] ],
    [ "UML Model auf the AudioManager", "uml.html", [
      [ "Audio Manager Branch", "uml.html#svn", null ]
    ] ],
    [ "AudioManager Components", "audiomanagercomponentspage.html", [
      [ "AudioManagerDaemon", "audiomanagercomponentspage.html#audiomanagercomponents", [
        [ "Daemon Overview", "audiomanagercomponentspage.html#daemonover", null ]
      ] ],
      [ "AudioManagerCommandPlugin", "audiomanagercomponentspage.html#commander", [
        [ "Interfaces", "audiomanagercomponentspage.html#commandIface", null ]
      ] ],
      [ "AudioManagerController", "audiomanagercomponentspage.html#controller", [
        [ "Interfaces", "audiomanagercomponentspage.html#controlIface", null ]
      ] ],
      [ "Routing AudioManagerRoutingPlugin", "audiomanagercomponentspage.html#router", [
        [ "Interfaces", "audiomanagercomponentspage.html#routingIface", null ],
        [ "Bus topology", "audiomanagercomponentspage.html#subrouter", null ],
        [ "Busname", "audiomanagercomponentspage.html#busname", null ],
        [ "CommonAPI plugins", "audiomanagercomponentspage.html#CAPIplugins", null ]
      ] ]
    ] ],
    [ "AudioManager and CommonAPI", "comminAPI.html", null ],
    [ "Elements of the AudioManagement", "elementspage.html", [
      [ "Overview Class Diagram", "elementspage.html#cDiag", null ],
      [ "Sources", "elementspage.html#source", [
        [ "Attributes", "elementspage.html#sourceattributes", null ]
      ] ],
      [ "Sinks", "elementspage.html#sinks", [
        [ "Attributes", "elementspage.html#sinkattributes", null ]
      ] ],
      [ "Gateways", "elementspage.html#gw", [
        [ "Attributes", "elementspage.html#gwattributes", null ]
      ] ],
      [ "Crossfaders", "elementspage.html#crossfaders", [
        [ "Attributes", "elementspage.html#cfattributes", null ]
      ] ]
    ] ],
    [ "The relation of sources & sinks with the AudioManager", "sourcesink.html", [
      [ "Class diagramm of the relation between sources, sinks and the AudioManager", "sourcesink.html#claDi", null ],
      [ "The REAL interaction", "sourcesink.html#boil", null ],
      [ "Connection Formats", "sourcesink.html#conFormats", null ],
      [ "Source States", "sourcesink.html#sstates", null ],
      [ "Availability", "sourcesink.html#avail", null ],
      [ "Volumes", "sourcesink.html#vol", null ],
      [ "SoundProperties", "sourcesink.html#SoundProperties", null ],
      [ "Interrupt States", "sourcesink.html#in", null ]
    ] ],
    [ "About unique IDs : Static vs Dynamic IDs", "uniquepage.html", [
      [ "Why having two different kinds of ids?", "uniquepage.html#why", null ],
      [ "The setup", "uniquepage.html#setup", null ]
    ] ],
    [ "Classification of Sinks and Sources", "classficationpage.html", [
      [ "Classification", "classficationpage.html#classification", null ],
      [ "Attributes", "classficationpage.html#attributes", null ]
    ] ],
    [ "Interrups & Low Level Interrupts", "interrupts.html", [
      [ "Differences", "interrupts.html#diff", null ],
      [ "Criterias", "interrupts.html#crit", null ]
    ] ],
    [ "Connections & MainConnections", "connpage.html", [
      [ "Connections", "connpage.html#con", null ],
      [ "Mainconnections", "connpage.html#maincon", null ],
      [ "Attributes", "connpage.html#att", null ]
    ] ],
    [ "Lipsync", "lip.html", [
      [ "The Task of the Audiomanager", "lip.html#t", null ],
      [ "Example", "lip.html#ex", null ]
    ] ],
    [ "Early Audio", "early.html", [
      [ "The Requirement", "early.html#req", null ],
      [ "Early Startup", "early.html#earlys", null ],
      [ "Late Rundown", "early.html#late", null ]
    ] ],
    [ "The two views of the AudioManager", "views.html", [
      [ "The CommandInterface View View", "views.html#command", null ],
      [ "RoutingInterface View", "views.html#route", null ],
      [ "Overview", "views.html#over", null ]
    ] ],
    [ "Volumes & MainVolumes", "vol.html", [
      [ "MainVolumes", "vol.html#mainVol", null ],
      [ "Volumes", "vol.html#volv", null ]
    ] ],
    [ "Properties", "prop.html", [
      [ "SoundProperties & MainSoundProperties", "prop.html#soundprop", null ],
      [ "SystemProperties", "prop.html#sys", null ]
    ] ],
    [ "Notifications", "notifi.html", [
      [ "What are notifications?", "notifi.html#notifi_ex", null ],
      [ "Overview", "notifi.html#notifi_overview", null ],
      [ "CommandInterface", "notifi.html#notifi_command", null ],
      [ "ControlInterface", "notifi.html#notifi_control", null ],
      [ "RoutingInterface", "notifi.html#notifi_routing", null ],
      [ "Notification Levels", "notifi.html#notifi_levels", null ]
    ] ],
    [ "Miscellaneous", "misc.html", [
      [ "Connection Formats", "misc.html#misc_connfor", null ],
      [ "Persistence", "misc.html#misc_pers", null ],
      [ "Speed dependent volume", "misc.html#misc_speed", null ]
    ] ],
    [ "Last User Mode", "luc.html", [
      [ "Last User Mode concept", "luc.html#luc_concept", null ],
      [ "The handling in the rundown context:", "luc.html#luc_rundown", null ],
      [ "The next startup:", "luc.html#luc_startup", null ]
    ] ],
    [ "Mainloop concept", "mainl.html", [
      [ "Mainloop", "mainl.html#mconcept", null ],
      [ "Using the Mainloop", "mainl.html#sec", null ],
      [ "Utilizing The Mainloop as Threadsafe Call Method", "mainl.html#util", [
        [ "Asynchronous calls", "mainl.html#async", null ],
        [ "Synchronous calls", "mainl.html#sync", null ]
      ] ]
    ] ],
    [ "The watchdog", "watchd.html", [
      [ "The watchdog concept", "watchd.html#watchdconcept", null ],
      [ "Watchdog configuration", "watchd.html#configwatch", null ],
      [ "Integration with systemd", "watchd.html#winteg", null ]
    ] ],
    [ "Startup and Rundown", "start.html", [
      [ "Startup", "start.html#start_Start", null ],
      [ "Rundown", "start.html#start_Rundown", null ],
      [ "Cancelled Rundown", "start.html#start_Cancel", null ]
    ] ],
    [ "CommandLineParsing", "cmdline.html", [
      [ "TCLAP", "cmdline.html#tclap", null ],
      [ "CommandLine Parsing in the Plugins", "cmdline.html#cmdplugins", null ]
    ] ],
    [ "Dlt support", "dlt.html", [
      [ "Compilerswitch", "dlt.html#compile", null ]
    ] ],
    [ "Download Compile Debug", "eclip.html", [
      [ "Get the source", "eclip.html#dw", null ],
      [ "Compile", "eclip.html#build", null ],
      [ "Using Eclipse", "eclip.html#ec", null ],
      [ "Debugging with eclipse", "eclip.html#deb", null ]
    ] ],
    [ "Compiling & Co", "comp.html", null ],
    [ "Namespaces", null, [
      [ "Namespace List", "namespaces.html", "namespaces" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", null, [
      [ "Class List", "annotated.html", "annotated" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", null, [
      [ "File List", "files.html", "files" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"AudioManagerCore_2include_2TAmPluginTemplate_8h.html",
"audiomanagertypes_8h.html#a58a230b5da10699a7ce1b1f2a1c550e6",
"classam_1_1CAmControlReceiver.html#a9d6eae2312f5629f748ec293ef8ef118",
"classam_1_1CAmDatabaseObserver.html#a3ab11ede15d50e01ed57d135192cc05d",
"classam_1_1CAmRoutingSender_1_1handleDisconnect.html#aca4d52488579bf52e45e313c7e6b351a",
"classam_1_1IAmControlReceive.html#a55b0c17d87150f44659bf2bb8a668408",
"classam_1_1IAmDatabaseHandler.html#af7e0be6ef2f4261a28b5eccc40202342",
"main_8cpp.html#acc628ca4f61759495f09289b8418d358",
"ver.html#new_ver"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
var navTreeSubIndices = new Array();

function getData(varName)
{
  var i = varName.lastIndexOf('/');
  var n = i>=0 ? varName.substring(i+1) : varName;
  return eval(n.replace(/\-/g,'_'));
}

function stripPath(uri)
{
  return uri.substring(uri.lastIndexOf('/')+1);
}

function stripPath2(uri)
{
  var i = uri.lastIndexOf('/');
  var s = uri.substring(i+1);
  var m = uri.substring(0,i+1).match(/\/d\w\/d\w\w\/$/);
  return m ? uri.substring(i-6) : s;
}

function localStorageSupported()
{
  try {
    return 'localStorage' in window && window['localStorage'] !== null && window.localStorage.getItem;
  }
  catch(e) {
    return false;
  }
}


function storeLink(link)
{
  if (!$("#nav-sync").hasClass('sync') && localStorageSupported()) {
      window.localStorage.setItem('navpath',link);
  }
}

function deleteLink()
{
  if (localStorageSupported()) {
    window.localStorage.setItem('navpath','');
  } 
}

function cachedLink()
{
  if (localStorageSupported()) {
    return window.localStorage.getItem('navpath');
  } else {
    return '';
  }
}

function getScript(scriptName,func,show)
{
  var head = document.getElementsByTagName("head")[0]; 
  var script = document.createElement('script');
  script.id = scriptName;
  script.type = 'text/javascript';
  script.onload = func; 
  script.src = scriptName+'.js'; 
  if ($.browser.msie && $.browser.version<=8) { 
    // script.onload does not work with older versions of IE
    script.onreadystatechange = function() {
      if (script.readyState=='complete' || script.readyState=='loaded') { 
        func(); if (show) showRoot(); 
      }
    }
  }
  head.appendChild(script); 
}

function createIndent(o,domNode,node,level)
{
  var level=-1;
  var n = node;
  while (n.parentNode) { level++; n=n.parentNode; }
  if (node.childrenData) {
    var imgNode = document.createElement("img");
    imgNode.style.paddingLeft=(16*level).toString()+'px';
    imgNode.width  = 16;
    imgNode.height = 22;
    imgNode.border = 0;
    node.plus_img = imgNode;
    node.expandToggle = document.createElement("a");
    node.expandToggle.href = "javascript:void(0)";
    node.expandToggle.onclick = function() {
      if (node.expanded) {
        $(node.getChildrenUL()).slideUp("fast");
        node.plus_img.src = node.relpath+"ftv2pnode.png";
        node.expanded = false;
      } else {
        expandNode(o, node, false, false);
      }
    }
    node.expandToggle.appendChild(imgNode);
    domNode.appendChild(node.expandToggle);
    imgNode.src = node.relpath+"ftv2pnode.png";
  } else {
    var span = document.createElement("span");
    span.style.display = 'inline-block';
    span.style.width   = 16*(level+1)+'px';
    span.style.height  = '22px';
    span.innerHTML = '&#160;';
    domNode.appendChild(span);
  } 
}

var animationInProgress = false;

function gotoAnchor(anchor,aname,updateLocation)
{
  var pos, docContent = $('#doc-content');
  if (anchor.parent().attr('class')=='memItemLeft' ||
      anchor.parent().attr('class')=='fieldtype' ||
      anchor.parent().is(':header')) 
  {
    pos = anchor.parent().position().top;
  } else if (anchor.position()) {
    pos = anchor.position().top;
  }
  if (pos) {
    var dist = Math.abs(Math.min(
               pos-docContent.offset().top,
               docContent[0].scrollHeight-
               docContent.height()-docContent.scrollTop()));
    animationInProgress=true;
    docContent.animate({
      scrollTop: pos + docContent.scrollTop() - docContent.offset().top
    },Math.max(50,Math.min(500,dist)),function(){
      if (updateLocation) window.location.href=aname;
      animationInProgress=false;
    });
  }
}

function newNode(o, po, text, link, childrenData, lastNode)
{
  var node = new Object();
  node.children = Array();
  node.childrenData = childrenData;
  node.depth = po.depth + 1;
  node.relpath = po.relpath;
  node.isLast = lastNode;

  node.li = document.createElement("li");
  po.getChildrenUL().appendChild(node.li);
  node.parentNode = po;

  node.itemDiv = document.createElement("div");
  node.itemDiv.className = "item";

  node.labelSpan = document.createElement("span");
  node.labelSpan.className = "label";

  createIndent(o,node.itemDiv,node,0);
  node.itemDiv.appendChild(node.labelSpan);
  node.li.appendChild(node.itemDiv);

  var a = document.createElement("a");
  node.labelSpan.appendChild(a);
  node.label = document.createTextNode(text);
  node.expanded = false;
  a.appendChild(node.label);
  if (link) {
    var url;
    if (link.substring(0,1)=='^') {
      url = link.substring(1);
      link = url;
    } else {
      url = node.relpath+link;
    }
    a.className = stripPath(link.replace('#',':'));
    if (link.indexOf('#')!=-1) {
      var aname = '#'+link.split('#')[1];
      var srcPage = stripPath($(location).attr('pathname'));
      var targetPage = stripPath(link.split('#')[0]);
      a.href = srcPage!=targetPage ? url : "javascript:void(0)"; 
      a.onclick = function(){
        storeLink(link);
        if (!$(a).parent().parent().hasClass('selected'))
        {
          $('.item').removeClass('selected');
          $('.item').removeAttr('id');
          $(a).parent().parent().addClass('selected');
          $(a).parent().parent().attr('id','selected');
        }
        var anchor = $(aname);
        gotoAnchor(anchor,aname,true);
      };
    } else {
      a.href = url;
      a.onclick = function() { storeLink(link); }
    }
  } else {
    if (childrenData != null) 
    {
      a.className = "nolink";
      a.href = "javascript:void(0)";
      a.onclick = node.expandToggle.onclick;
    }
  }

  node.childrenUL = null;
  node.getChildrenUL = function() {
    if (!node.childrenUL) {
      node.childrenUL = document.createElement("ul");
      node.childrenUL.className = "children_ul";
      node.childrenUL.style.display = "none";
      node.li.appendChild(node.childrenUL);
    }
    return node.childrenUL;
  };

  return node;
}

function showRoot()
{
  var headerHeight = $("#top").height();
  var footerHeight = $("#nav-path").height();
  var windowHeight = $(window).height() - headerHeight - footerHeight;
  (function (){ // retry until we can scroll to the selected item
    try {
      var navtree=$('#nav-tree');
      navtree.scrollTo('#selected',0,{offset:-windowHeight/2});
    } catch (err) {
      setTimeout(arguments.callee, 0);
    }
  })();
}

function expandNode(o, node, imm, showRoot)
{
  if (node.childrenData && !node.expanded) {
    if (typeof(node.childrenData)==='string') {
      var varName    = node.childrenData;
      getScript(node.relpath+varName,function(){
        node.childrenData = getData(varName);
        expandNode(o, node, imm, showRoot);
      }, showRoot);
    } else {
      if (!node.childrenVisited) {
        getNode(o, node);
      } if (imm || ($.browser.msie && $.browser.version>8)) { 
        // somehow slideDown jumps to the start of tree for IE9 :-(
        $(node.getChildrenUL()).show();
      } else {
        $(node.getChildrenUL()).slideDown("fast");
      }
      if (node.isLast) {
        node.plus_img.src = node.relpath+"ftv2mlastnode.png";
      } else {
        node.plus_img.src = node.relpath+"ftv2mnode.png";
      }
      node.expanded = true;
    }
  }
}

function glowEffect(n,duration)
{
  n.addClass('glow').delay(duration).queue(function(next){
    $(this).removeClass('glow');next();
  });
}

function highlightAnchor()
{
  var aname = $(location).attr('hash');
  var anchor = $(aname);
  if (anchor.parent().attr('class')=='memItemLeft'){
    var rows = $('.memberdecls tr[class$="'+
               window.location.hash.substring(1)+'"]');
    glowEffect(rows.children(),300); // member without details
  } else if (anchor.parents().slice(2).prop('tagName')=='TR') {
    glowEffect(anchor.parents('div.memitem'),1000); // enum value
  } else if (anchor.parent().attr('class')=='fieldtype'){
    glowEffect(anchor.parent().parent(),1000); // struct field
  } else if (anchor.parent().is(":header")) {
    glowEffect(anchor.parent(),1000); // section header
  } else {
    glowEffect(anchor.next(),1000); // normal member
  }
  gotoAnchor(anchor,aname,false);
}

function selectAndHighlight(hash,n)
{
  var a;
  if (hash) {
    var link=stripPath($(location).attr('pathname'))+':'+hash.substring(1);
    a=$('.item a[class$="'+link+'"]');
  }
  if (a && a.length) {
    a.parent().parent().addClass('selected');
    a.parent().parent().attr('id','selected');
    highlightAnchor();
  } else if (n) {
    $(n.itemDiv).addClass('selected');
    $(n.itemDiv).attr('id','selected');
  }
  if ($('#nav-tree-contents .item:first').hasClass('selected')) {
    $('#nav-sync').css('top','30px');
  } else {
    $('#nav-sync').css('top','5px');
  }
  showRoot();
}

function showNode(o, node, index, hash)
{
  if (node && node.childrenData) {
    if (typeof(node.childrenData)==='string') {
      var varName    = node.childrenData;
      getScript(node.relpath+varName,function(){
        node.childrenData = getData(varName);
        showNode(o,node,index,hash);
      },true);
    } else {
      if (!node.childrenVisited) {
        getNode(o, node);
      }
      $(node.getChildrenUL()).css({'display':'block'});
      if (node.isLast) {
        node.plus_img.src = node.relpath+"ftv2mlastnode.png";
      } else {
        node.plus_img.src = node.relpath+"ftv2mnode.png";
      }
      node.expanded = true;
      var n = node.children[o.breadcrumbs[index]];
      if (index+1<o.breadcrumbs.length) {
        showNode(o,n,index+1,hash);
      } else {
        if (typeof(n.childrenData)==='string') {
          var varName = n.childrenData;
          getScript(n.relpath+varName,function(){
            n.childrenData = getData(varName);
            node.expanded=false;
            showNode(o,node,index,hash); // retry with child node expanded
          },true);
        } else {
          var rootBase = stripPath(o.toroot.replace(/\..+$/, ''));
          if (rootBase=="index" || rootBase=="pages" || rootBase=="search") {
            expandNode(o, n, true, true);
          }
          selectAndHighlight(hash,n);
        }
      }
    }
  } else {
    selectAndHighlight(hash);
  }
}

function removeToInsertLater(element) {
  var parentNode = element.parentNode;
  var nextSibling = element.nextSibling;
  parentNode.removeChild(element);
  return function() {
    if (nextSibling) {
      parentNode.insertBefore(element, nextSibling);
    } else {
      parentNode.appendChild(element);
    }
  };
}

function getNode(o, po)
{
  var insertFunction = removeToInsertLater(po.li);
  po.childrenVisited = true;
  var l = po.childrenData.length-1;
  for (var i in po.childrenData) {
    var nodeData = po.childrenData[i];
    po.children[i] = newNode(o, po, nodeData[0], nodeData[1], nodeData[2],
      i==l);
  }
  insertFunction();
}

function gotoNode(o,subIndex,root,hash,relpath)
{
  var nti = navTreeSubIndices[subIndex][root+hash];
  o.breadcrumbs = $.extend(true, [], nti ? nti : navTreeSubIndices[subIndex][root]);
  if (!o.breadcrumbs && root!=NAVTREE[0][1]) { // fallback: show index
    navTo(o,NAVTREE[0][1],"",relpath);
    $('.item').removeClass('selected');
    $('.item').removeAttr('id');
  }
  if (o.breadcrumbs) {
    o.breadcrumbs.unshift(0); // add 0 for root node
    showNode(o, o.node, 0, hash);
  }
}

function navTo(o,root,hash,relpath)
{
  var link = cachedLink();
  if (link) {
    var parts = link.split('#');
    root = parts[0];
    if (parts.length>1) hash = '#'+parts[1];
    else hash='';
  }
  if (hash.match(/^#l\d+$/)) {
    var anchor=$('a[name='+hash.substring(1)+']');
    glowEffect(anchor.parent(),1000); // line number
    hash=''; // strip line number anchors
    //root=root.replace(/_source\./,'.'); // source link to doc link
  }
  var url=root+hash;
  var i=-1;
  while (NAVTREEINDEX[i+1]<=url) i++;
  if (i==-1) { i=0; root=NAVTREE[0][1]; } // fallback: show index
  if (navTreeSubIndices[i]) {
    gotoNode(o,i,root,hash,relpath)
  } else {
    getScript(relpath+'navtreeindex'+i,function(){
      navTreeSubIndices[i] = eval('NAVTREEINDEX'+i);
      if (navTreeSubIndices[i]) {
        gotoNode(o,i,root,hash,relpath);
      }
    },true);
  }
}

function showSyncOff(n,relpath)
{
    n.html('<img src="'+relpath+'sync_off.png" title="'+SYNCOFFMSG+'"/>');
}

function showSyncOn(n,relpath)
{
    n.html('<img src="'+relpath+'sync_on.png" title="'+SYNCONMSG+'"/>');
}

function toggleSyncButton(relpath)
{
  var navSync = $('#nav-sync');
  if (navSync.hasClass('sync')) {
    navSync.removeClass('sync');
    showSyncOff(navSync,relpath);
    storeLink(stripPath2($(location).attr('pathname'))+$(location).attr('hash'));
  } else {
    navSync.addClass('sync');
    showSyncOn(navSync,relpath);
    deleteLink();
  }
}

function initNavTree(toroot,relpath)
{
  var o = new Object();
  o.toroot = toroot;
  o.node = new Object();
  o.node.li = document.getElementById("nav-tree-contents");
  o.node.childrenData = NAVTREE;
  o.node.children = new Array();
  o.node.childrenUL = document.createElement("ul");
  o.node.getChildrenUL = function() { return o.node.childrenUL; };
  o.node.li.appendChild(o.node.childrenUL);
  o.node.depth = 0;
  o.node.relpath = relpath;
  o.node.expanded = false;
  o.node.isLast = true;
  o.node.plus_img = document.createElement("img");
  o.node.plus_img.src = relpath+"ftv2pnode.png";
  o.node.plus_img.width = 16;
  o.node.plus_img.height = 22;

  if (localStorageSupported()) {
    var navSync = $('#nav-sync');
    if (cachedLink()) {
      showSyncOff(navSync,relpath);
      navSync.removeClass('sync');
    } else {
      showSyncOn(navSync,relpath);
    }
    navSync.click(function(){ toggleSyncButton(relpath); });
  }

  $(window).load(function(){
    navTo(o,toroot,window.location.hash,relpath);
    showRoot();
  });

  $(window).bind('hashchange', function(){
     if (window.location.hash && window.location.hash.length>1){
       var a;
       if ($(location).attr('hash')){
         var clslink=stripPath($(location).attr('pathname'))+':'+
                               $(location).attr('hash').substring(1);
         a=$('.item a[class$="'+clslink+'"]');
       }
       if (a==null || !$(a).parent().parent().hasClass('selected')){
         $('.item').removeClass('selected');
         $('.item').removeAttr('id');
       }
       var link=stripPath2($(location).attr('pathname'));
       navTo(o,link,$(location).attr('hash'),relpath);
     } else if (!animationInProgress) {
       $('#doc-content').scrollTop(0);
       $('.item').removeClass('selected');
       $('.item').removeAttr('id');
       navTo(o,toroot,window.location.hash,relpath);
     }
  })
}

