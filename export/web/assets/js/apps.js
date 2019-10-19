function Applications() {
    this.my_app = null;
    this.apps = null;
    this.nodeTypes = [];
    this.newtype_keys = [];

    if (has_app_root == 1) {

        $('#app_root_error').css('display', 'none');
        $('#app_root_new').css('display', 'none');
        $('#app_root_new').html('');
        $('#app_root_infos').css('display', 'block');
        $('#app_root_infos').html('<div class="row"><div class="col-md">txid</div><div class="col-md"  id="root_app_txh"></div></div><div class="row"><div class="col-md">addr</div><div class="col-md" id="root_app_addr"></div></div>');
             

        $('#root_app_txh').html(root_app_hash);
        $('#root_app_addr').html(root_app_addr);
        $('#root_app_fees').html(root_app_fees / unit);
        $('#totalfee').html((paytxfee + root_app_fees) / unit);
    }
    else {

        if (typeof select_addr != 'undefined')
            MyAccount.addr_selected = select_addr;

        $('#app_root_error').css('display', 'block');
        $('#app_root_infos').css('display', 'none');
        $('#app_root_infos').html('');
        $('#app_root_new').css('display', 'block');
        $('#app_root_new').html('<section><h2>New app root</h2><div id="address_list" class="table"></div><div class="container"><fieldset><legend>no app root set</legend><div class="row"><div class="col-md-2"><label>select an address to use as approot</label></div><div class="col-md-2"><select style="width:200px;" name="new_app_addr" id="new_app_addr" class="browser-default"></select></div></div><div class="row"><div class="col-md-2"><label>Set application fee amount</label></div><div class="col-md-2"><input type="text" name="new_app_fee" id="new_app_fee" /></div></div><div class="row"><div class="col-md-2"><input type="button" value="set app root" name="new_app_submit" id="new_app_submit" onclick="MyApps.set_app_root();" /></div></div></fieldset></div></section >');
    }
}

Applications.prototype.update_type_keys = function (list_id)
{
   $('#' + list_id).empty();
   var list = document.getElementById(list_id);

   for(var n=0;n<this.newtype_keys.length;n++)
   {
       var name = this.get_obj_type_name(this.newtype_keys[n].key);
       if (name == null) continue;
       
        var row, col;

        row = document.createElement('div');
        row.className = "row";

            col = document.createElement('div');
            col.innerHTML = this.newtype_keys[n].name + ':' + name ;
            col.className = "col-md-2";
        row.appendChild(col);

        col = document.createElement('div');

        if (this.newtype_keys[n].unique == true) 
            col.innerHTML = 'unique';
        else if (this.newtype_keys[n].index == true) 
            col.innerHTML = 'index';
        else
            col.innerHTML = 'none';

        col.className = "col-md-2";
        row.appendChild(col);
        list.appendChild(row);
   }
}

Applications.prototype.get_type_input = function (typeid, key) {
    var id = typeid + '_' + key.name;

    if ((key.id >> 24) == 0x1E) {
        return '<input size="64" type="text" id="' + id + '" name="' + id + '" />';
    }
    else if ((key.id == 1024) || (key.id == 0x0A000040)) {
        return '<input size="64" type="text" id="' + id + '" name="' + id + '" />';
    }
    else if (key.id == 0x0A000100) {
        return '<input size="66" type="text" id="' + id + '" name="' + id + '" />';
    }
    else if ((key.id == 1) || (key.id == 0x0B000100)) {
        return '<input size="16" type="text" id="' + id + '" name="' + id + '" />';
    }
    else if ((key.id == 2) || (key.id == 8) || (key.id == 256)) {
        return '<input size="4" type="number" step="1" id="' + id + '" name="' + id + '">';
    }
    else if ((key.id == 4096) || (key.id == 16384)) {
        return '<input size="4" type="number"  id="' + id + '" name="' + id + '">';
    }
    else if (key.id == 218103810) {
        return '<input size="4" type="number" id="' + id + '_0" name="' + id + '_0"><input size="4" type="number" id="' + id + '_1" name="' + id + '_1"><input size="4"  type="number" id="' + id + '_2" name="' + id + '_2">';
    }
    else if (key.id == 0x00000080) {
        return '<input size="4" type="number" id="' + id + '_0" name="' + id + '_0"><input size="4" type="number" id="' + id + '_1" name="' + id + '_1"><input size="4"  type="number" id="' + id + '_2" name="' + id + '_2"><input size="4"  type="number" id="' + id + '_3" name="' + id + '_3">';
    }
}

Applications.prototype.get_type_input_val = function (typeid, key) {
    var id = typeid + '_' + key.name;

    if ((key.id >> 24) == 0x1E) {
        return $('#' + id).val();
    }
    else if ((key.id == 1) || (key.id == 0x0B000100) || (key.id == 0x0A000040)) {
        return $('#' + id).val();
    }
    else if (key.id == 0x0A000100) {
        return $('#' + id).val();
    }
    else if ((key.id == 2) || (key.id == 8) || (key.id == 256) || (key.id == 2048)) {
        return parseInt($('#' + id).val());
    }
    else if ((key.id == 4096) || (key.id == 16384)) {
        return parseFloat($('#' + id).val());
    }
    else if (key.id == 218103810) {

        var ret = [];
        ret.push(parseFloat($('#' + id + '_0').val()));
        ret.push(parseFloat($('#' + id + '_1').val()));
        ret.push(parseFloat($('#' + id + '_2').val()));

        return ret;
    }
    else if (key.id == 0x00000080) {

        var ret = [];
        ret.push(Math.min(255, parseInt($('#' + id + '_0').val())));
        ret.push(Math.min(255, parseInt($('#' + id + '_1').val())));
        ret.push(Math.min(255, parseInt($('#' + id + '_2').val())));
        ret.push(Math.min(255, parseInt($('#' + id + '_3').val())));
        return ret;
    }
    else if ((key.id == 1024) || (key.id == 0x0A000040)) {

        return '0';
    }
}

    
Applications.prototype.update_app_types = function () {
   var n, nn;
   var html = 'new obj address <select name="obj_addr" id="obj_addr" class="browser-default" onchange="$(\'.sel_obj_addr\').html($(this).val());"></select>';

   if (this.my_app.app_types == null)
       return;

   for (n = 0; n < this.my_app.app_types.length; n++) {
       html += '<div class="card">';
       html += '<div class="card-header indigo flex-center">';
       html += '<h3>' + this.my_app.app_types[n].name + '</h3>'
       html += '&nbsp;id :' + this.my_app.app_types[n].id.toString(16);
       html += '</div>';
       html += '<div class="card-body">';
       html += '<h4>keys</h4>'

       html += '<div class="container">';
       for (nn = 0; nn < this.my_app.app_types[n].keys.length; nn++) {

           html += '<div class="row">';
           html += '<div class="col-md-1">#' + nn + '</div>';
           html += '<div class="col-md-2">' + this.my_app.app_types[n].keys[nn].name + '</div>';
           html += '<div class="col-md-2">' + this.get_obj_type_name(this.my_app.app_types[n].keys[nn].id) + '</div>';
           html += '<div class="col-md-4">' + this.get_type_input(this.my_app.app_types[n].id, this.my_app.app_types[n].keys[nn]) + '</div>';
           html += '</div>';
       }


       html += '<div class="row">';
       html += '<div class="col-md-1"></div>';
       html += '<div class="col-md-4"><span class="sel_obj_addr"></span></div>';
       html += '<div class="col-md-4"><input type="button" onclick="var obj = MyApps.new_obj(' + this.my_app.app_types[n].id + '); MyApps.create_app_obj(' + this.my_app.app_types[n].id + ', $(\'#obj_addr\').val(), obj ,$(\'#paytxfee\').val());" value="new obj" /></div>';
       html += '</div>';
       html += '</div>';

       html += '<h4>objects</h4>'
       html += '<div class="container">';
       html += '<div id = "objs_' + this.my_app.app_types[n].id.toString(16) + '"></div>';
       html += '<div class="row"><div class="col-md-4"><div id="objData_' + this.my_app.app_types[n].id.toString(16) + '"></div></div></div>';
       html += '</div>';


       html += '<div id="childDiv_' + this.my_app.app_types[n].id.toString(16) + '">';
       html += '<h5>Children</h5>';
       html += '<div class="container">';
       html += '<div class="row">';
       html += '<div class="col-md-4"><span class="objid" id="pObj_' + this.my_app.app_types[n].id.toString(16) + '"></span></div>';
       html += '</div>';

       html += '<div class="row">';
       html += '<div class="col-md-2"><select id="objChildKeys_' + this.my_app.app_types[n].id.toString(16) + '" class="browser-default"></select></div>';
       html += '<div class="col-md-4">child object : <input style="display:inline;" id="objChildId_' + this.my_app.app_types[n].id.toString(16) + '"/></div>';
       html += '<div class="col-md-2"><input type="button" onclick="MyApps.add_app_obj_child( $(\'#pObj_' + this.my_app.app_types[n].id.toString(16) + '\').html(), $(\'#objChildKeys_' + this.my_app.app_types[n].id.toString(16) + '\').val(), $(\'#objChildId_' + this.my_app.app_types[n].id.toString(16) + '\').val(), $(\'#paytxfee\').val());" value="add child" /></div>';
       html += '</div>';
       html += '<div class="row">';
       html += '<div class="col-md-4"><div id="objChilds_' + this.my_app.app_types[n].id.toString(16) + '"></div></div>';
       html += '</div>';
       html += '</div>';
       html += '</div>';
       html += '</div>';

       html += '<hr/>';
   }

   $('#' + this.my_app.txid + '_types').html(html);
}
    
Applications.prototype.update_app_type_obj = function (typeId, objs) {
   var n;
   var self = this;
   var type = null;
   var tstr = typeId.toString(16);
   var has_childs;
      
   
   $('#objChildKeys_' + tstr).empty();
   for (n = 0; n < this.my_app.app_types.length; n++) {
       if (this.my_app.app_types[n].id == typeId)
           type = this.my_app.app_types[n];
   }

   if (type == null) return;

   has_childs = false;

   for (n = 0; n < type.keys.length; n++) {
       if (type.keys[n].id == 0x09000001) {
           $('#objChildKeys_' + tstr).append('<option>' + type.keys[n].name + '</option>');
           has_childs = true;
       }
   }

   if (has_childs)
       $('#childDiv_' + tstr).css('display', 'block');
   else
       $('#childDiv_' + tstr).css('display', 'none');

   $('#objs_' + tstr).empty();

   var objsDiv = document.getElementById('objs_' + tstr);
   var objdata = 'objData_' + tstr;
   var pobj = 'pObj_' + tstr;


   for (n = 0; n < objs.length; n++) {
       var mydiv = document.createElement('div');
       mydiv.className = "objid";
       mydiv.setAttribute('objId', objs[n]);
       mydiv.innerHTML = objs[n];
       mydiv.addEventListener("click", function () { self.load_obj(this.getAttribute('objId'), objdata); $('#'+pobj ).html( objs[n] ); });
       objsDiv.appendChild(mydiv);
   }
}

Applications.prototype.get_file_html = function (file,filedivname)
{
    var size = file.end - file.start;;
    var filediv = document.getElementById(filedivname);
    var row, col;

    $('#' + filedivname).empty();
             
    if (!file.filename)
        file.filename = 'file';


    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.innerHTML = 'name :';
        col.className = "col-md-2";
    row.appendChild(col);

        col = document.createElement('div');
        col.innerHTML = file.filename;
        col.className = "col-md-6";
    row.appendChild(col);
    filediv.appendChild(row);

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.innerHTML = 'mime :';
        col.className = "col-md-2";
    row.appendChild(col);

        col = document.createElement('div');
        col.innerHTML = file.mime;
        col.className = "col-md-6";
    row.appendChild(col);
    filediv.appendChild(row);

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.innerHTML = 'size :';
        col.className = "col-md-2";
     row.appendChild(col);

        col = document.createElement('div');
        col.innerHTML = file.size;
        col.className = "col-md-6";
    row.appendChild(col);
    filediv.appendChild(row);

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.innerHTML = 'hash :';
        col.className = "col-md-2";
    row.appendChild(col);

        col = document.createElement('div');
        col.innerHTML = file.dataHash;
        col.className = "col-md-6";
    row.appendChild(col);
    filediv.appendChild(row);
    return filediv;
}

Applications.prototype.update_app_file = function (file, path) {
    var filediv = this.get_file_html(file, this.my_app.txid + '_file');
    var row, col;

   row = document.createElement('div');
   row.className = "row";

       col = document.createElement('div');
       col.className = "col-md-2";
       col.innerHTML = 'url :';
   row.appendChild(col);

       col = document.createElement('div');
       col.className = "col-md-2";
       col.innerHTML = '<a href="' + path + '" >' + file.filename + '</a>';
   row.appendChild(col);

   filediv.appendChild(row);
}
    
Applications.prototype.update_app_layout = function (file, path) {
    var layoutdiv = this.get_file_html(file, this.my_app.txid + '_layout');
    var row, col;

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.className = "col-md-2";
        col.innerHTML = 'url :';
    
    row.appendChild(col);

        col = document.createElement('div');
        col.className = "col-md-2";
        col.innerHTML = '<a href="' + path + '" >' + file.filename + '</a>';
    row.appendChild(col);

    layoutdiv.appendChild(row);
}
    
Applications.prototype.update_app_upl = function (file) {
    var self = this;
    var upldiv = this.get_file_html(file, 'upl_file');
    var row, col, input;

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.className = "col-md-2";
        col.innerHTML = 'addr :';
    row.appendChild(col);

        col = document.createElement('div');
        col.className = "col-md-2";
        col.innerHTML = '<select name="fileKey" id="fileKey" class="browser-default" ><option>no addr</option></select>';
    row.appendChild(col);

    upldiv.appendChild(row);

    row = document.createElement('div');
    row.className = "row";

        col            = document.createElement('div');
        col.className  = "col-md-4";

        input          = document.createElement('input');
        input.type     = "button";
        input.value = "create file tx";
        input.setAttribute('dataHash', file.dataHash);

        input.addEventListener("click", function () { self.create_app_file( this.getAttribute('dataHash') , $('#fileKey').val(), $('#paytxfee').val()); });

        col.appendChild(input);

    row.appendChild(col);

    upldiv.appendChild(row);
}
    
Applications.prototype.update_app_upl_layout = function (file) {
     var self = this;
     var upllayoutdiv = this.get_file_html(file, 'upl_layout');
     var row, col, input;

     row = document.createElement('div');
     row.className = "row";

     col = document.createElement('div');
     col.className = "col-md-4";

     input = document.createElement('input');
     input.type = "button";
     input.value = "create layout tx";
     input.setAttribute('dataHash', file.dataHash);

     input.addEventListener("click", function () { self.create_app_layout(this.getAttribute('dataHash'), $('#paytxfee').val() ); });

     col.appendChild(input);
     row.appendChild(col);

     upllayoutdiv.appendChild(row);
}
    
Applications.prototype.update_app_upl_module = function (file) {
    var self = this;
    var uplmoddiv = this.get_file_html(file, 'upl_mod');
    var row, col,input;

    row = document.createElement('div');
    row.className = "row";

        col = document.createElement('div');
        col.className = "col-md-4";

        input = document.createElement('input');
        input.type = "button";
        input.value = "create module tx";
        input.setAttribute('dataHash', file.dataHash);

        input.addEventListener("click", function () { self.create_app_module(this.getAttribute('dataHash'), $('#paytxfee').val()) });

       col.appendChild(input);
       row.appendChild(col);

    uplmoddiv.appendChild(row);    

}

Applications.prototype.update_app_files = function (files, numFiles) {
    var self = this;

    $('#' + this.my_app.txid + '_files').empty();

    var uplappfilediv = document.getElementById(this.my_app.txid + '_files');

    if (uplappfilediv == null) return;

    for (var n = 0; n < files.length; n++) {
        var row, col, span;
        var myfile;
        row = document.createElement('div');
        row.className = "row";

        span = document.createElement('span');
        span.className = "fileid";
        span.innerHTML = files[n];
        span.addEventListener("click", function () { self.get_app_file(this.innerHTML); });

        col = document.createElement('div');
        col.className = "col-md";

        col.appendChild(span);
        row.appendChild(col);

        uplappfilediv.appendChild(row);
    }
}


Applications.prototype.get_app_header = function (app) {
    var section,h1, div,inner, cont,row,col;

    section = document.createElement('section');

    h1 = document.createElement('h1');
    div = document.createElement('div');
    div.className = 'card';

    inner = document.createElement('div');
    inner.className = 'card-header  pt-3 aqua-gradient';

     if (app.locked)
        h1.innerHTML = '<i class="fa fa-lock"></i><a href="/nodix.site/application/' + app.appName + '" >' + app.appName + '</a>';
    else
        h1.innerHTML = '<a href="/nodix.site/application/' + app.appName + '" >' + app.appName + '</a>';
        

    inner.appendChild(h1);
    div.appendChild(inner);

    inner = document.createElement('div');
    inner.className = 'card-body px-lg-5 pt-0';

    cont = document.createElement('div');
    cont.className = 'container';
    row = document.createElement('div');
    row.className = 'row';
    col = document.createElement('div');
    col.className = 'col-md-2';
    col.innerHTML = 'Master addr';
    row.appendChild(col);
    col = document.createElement('div');
    col.className = 'col hash-lnk';
    col.id = 'appAddr';
    col.innerHTML = app.appAddr;
    row.appendChild(col);
    cont.appendChild(row);
    
    row = document.createElement('div');
    row.className = 'row';
    col = document.createElement('div');
    col.className = 'col-md-2';
    col.innerHTML = 'txid';
    row.appendChild(col);
    col = document.createElement('div');
    col.className = 'col hash-lnk';
    col.innerHTML = app.txid;
    row.appendChild(col);
    cont.appendChild(row);

    
    inner.appendChild(cont);
    div.appendChild(inner);
    section.appendChild(div);

    return section;
}

Applications.prototype.get_app_items = function (section, div_id, label, html) {
    var pane = document.createElement('div');
    
    pane.className = 'tab-pane fade in';
    pane.id = div_id;
    pane.setAttribute('role', 'tabpanel');
    pane.innerHTML = html;
    
    section.appendChild(pane);
}
    
Applications.prototype.get_app_section = function () {
   var types, objects;
   var html;
   var section, h2, appdiv;

   section = document.createElement('div');
   section.className = 'tab-content card';
   
   /**/       
   html =  '<div id="' + this.my_app.txid + '_types" ></div>';
   html += '<div class="container" id="new_type">';
   html += '<fieldset><legend>new type</legend>';
   html += '<div class="row"><div class="col-md-2">name : <input type="text" size="12" name="new_type_name" id="new_type_name" /></div><div class="col-md-2">id : <input type="text" size="6" name="new_type_id" id="new_type_id" /></div></div>';
   html += '<hr/>';
   html += '<h3>keys</h3>';
   html += '<div id="new_type_keys"></div>';
   html += '<hr/>';
   html += '<div class="row"><div class="col"><input type="button" onclick="MyApps.create_type();" value="new type" /></div></div>';
   html += '<hr/>';
   html += '<div class="container">';
   html += '<h5>new key</h5>';
   html += '<div class="row">';
   html += '<div class="col-md-2"><label for="new_type_key_name">name</label><input type="text" size="12" name="new_type_key_name" id="new_type_key_name" /></div>';
   html += '<div class="col-md-3"><div class="row"><div class="col-md-2" ><label for="new_type_key">type</label></div><div class="col-md-2" ><select  name="new_type_key" id="new_type_key" class="browser-default"></select></div></div></div>';
   html += '<div class="col-sm-1"><div class="custom-control custom-radio-inline"><input type="radio" class="custom-control-input" id="new_type_key_flags_1" checked="checked" name="new_type_key_flags" value="0" /><label class="custom-control-label" for="new_type_key_flags_1">none</label></div></div>';
   html += '<div class="col-sm-1"><div class="custom-control custom-radio-inline"><input type="radio" class="custom-control-input" id="new_type_key_flags_2" name="new_type_key_flags" value="1" /><label class="custom-control-label" for="new_type_key_flags_2">unique</label></div></div>';
   html += '<div class="col-sm-1"><div class="custom-control custom-radio-inline"><input type="radio" class="custom-control-input" id="new_type_key_flags_3" name="new_type_key_flags" value="2" /><label class="custom-control-label" for="new_type_key_flags_3">index</label></div></div>';
   html += '<div class="col-sm-1"><input type="button" onclick="if ( MyApps.add_type_key( $(\'#new_type_key_name\').val(), $(\'#new_type_key\').val(), $(\'input[name = new_type_key_flags]:checked\').val() )) { $(\'#new_type_key_name\').val(\'\'); } "value="+" /></div>';
   html += '</div>';
   html += '</div>';
   html += '</fieldset>';
   html += '<hr/>';
   html += '</div>';

   this.get_app_items(section, 'app_type_div', 'types', html);

   /**/
   html = '<div id="' + this.my_app.txid + '_files"></div>';
   html += '<div id="' + this.my_app.txid + '_file"></div>';
   html += '<form method="POST" enctype="multipart/form-data">';
   html += '<fieldset><legend>new file</legend>';
   html += '<div id="upl_file"></div>';
   html += '<div id="file_error"></div>';
   html += '<div class="row"><div class="col-md-2"><input type="file" name="myfile" id="myfile" value="new file" /></div><div class="col-md-2"><input type="submit" value="send" /></div></div>';
   html += '</fieldset>';
   html += '</form>';
   html += '<hr/>';

   this.get_app_items(section, 'app_file_div', 'files', html);

    /**/

   html =  '<div id="' + this.my_app.txid + '_layouts"></div>';
   html += '<div id="' + this.my_app.txid + '_layout"></div>';
   html += '<form method="POST" enctype="multipart/form-data">';
   html += '<fieldset><legend>new layout</legend>';
   html += '<div id="upl_layout"></div>';
   html += '<div id="layout_error"></div>';
   html += '<div class="row"><div class="col-md-2"><input type="file" name="mylayout" id="mylayout" value="new layout" /></div><div class="col-md-2"><input type="submit" value="send" /></div></div>';
   html += '</fieldset>';
   html += '</form>';
   html += '<hr/>';

   this.get_app_items(section, 'app_layout_div', 'layouts', html);

   /**/
   html = '<div id="' + this.my_app.txid + '_mods"></div>';
   html += '<div id="' + this.my_app.txid + '_mod"></div>';
   html += '<form method="POST" enctype="multipart/form-data">';
   html += '<fieldset><legend>new module</legend>';
   html += '<div id="upl_mod"></div>';
   html += '<div id="mod_error"></div>';
   html += '<div class="row"><div class="col-md-2"><input type="file" name="mymod" id="mymod" value="new mod" /></div><div class="col-md-2"><input type="submit" value="send" /></div></div>';
   html += '</fieldset>';
   html += '</form>';

   this.get_app_items(section, 'app_mod_div', 'Modules & script', html);

   return section;
}
    
Applications.prototype.update_types_select = function (select_id) {
    var n;

    if (this.nodeTypes == null)
        return;

    $('#' + select_id).empty();
    for (n = 0; n < this.nodeTypes.length; n++) {
        $('<option value="' + this.nodeTypes[n].id + '">' + this.nodeTypes[n].name + '</option>').appendTo('#' + select_id);
    }

    if (this.my_app == null) return;
    
    for (n = 0; n < this.my_app.app_types.length; n++) {
        $('<option value="' + this.my_app.app_types[n].id + '">' + this.my_app.app_types[n].name + '</option>').appendTo('#' + select_id);
    }
}

Applications.prototype.add_type_key = function (name, key, unique) {
    var type = {};
    var n;

    
    $('#app_error').css('display', 'none');

    if (name.length < 3) {
        $('#app_error').html('type key must be 3 char long min.');
        $('#app_error').css('display', 'block');
        return false;
    }

    for (n = 0; n < this.newtype_keys.length; n++) {

        if (this.newtype_keys[n].name == name) {
            $('#app_error').html('type key already exists');
            $('#app_error').css('display', 'block');
            return false;
        }
    }

    type.name = name;
    type.key = parseInt(key);

    if (unique == 1)
        type.unique = true;
    else
        type.unique = false;

    if (unique == 2)
        type.index = true;
    else
        type.index = false;

    this.newtype_keys.push(type);
    this.update_type_keys('new_type_keys');

    return true;
}

Applications.prototype.create_type = function () {
    var type_name = $('#new_type_name').val();
    var type_id = parseInt($('#new_type_id').val());
    
    $('#app_error').css('display', 'none');
    
    if (type_name.length < 3) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('type name must be 3 char long min.');
        return false;
    }
    
    if ((type_id < 1) || (isNaN(type_id))) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('type id be a number >= 1.');
        return false;
    }
    
    if (this.newtype_keys.length < 1) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('type must have at least one key.');
        return false;
    }
    
    var addrEntry = document.getElementById('selected_' + this.my_app.appAddr);
    if (addrEntry == null) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('Select the account containing the app master key');
        return;
    }
    var DecHexkey = $(addrEntry).attr('privkey');
    if ((DecHexkey == null) || (DecHexkey.length < 64)) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('enter secret key of app master key');
        return;
    }
    
    var pubKey = $(addrEntry).attr('pubkey');
    var mykey = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
    var Cpubkey = mykey.getPublic().encodeCompressed('hex');
    var self = this;
    
    rpc_call('pubkeytoaddr', [pubKey], function (data) {
    
        if (Cpubkey != pubKey) {
            $('#app_error').css('display', 'block');
            $('#app_error').html('invalid secret key of app master key');
        }
        else {
            $('#appAddr').css("color", "green");
    
            self.create_app_type(type_name, type_id, parseInt($('#paytxfee').val() * unit));
        }
    });
}

Applications.prototype.new_obj = function (type_id) {
    var n, nn;
    var newobj = {};
    
    for (n = 0; n < this.my_app.app_types.length; n++) {
        if (this.my_app.app_types[n].id == type_id) {
            for (nn = 0; nn < this.my_app.app_types[n].keys.length; nn++) {
                var keyname = this.my_app.app_types[n].keys[nn].name;
                var id = type_id + '_' + keyname;
                var val = this.get_type_input_val(type_id, this.my_app.app_types[n].keys[nn]);
                newobj[keyname] = val;
            }
            return newobj;
        }
    }
    return null;
}

Applications.prototype.set_app_root = function () {
    var addr = $('#new_app_addr').val();
    var fees = $('#new_app_fee').val();

    rpc_call('create_root_app', [addr, fees], function (data) {
         if (!data.error) {
             $('#app_root_new').css('display', 'none');
             $('#app_root_infos').css('display', 'block');
             $('#root_app_txh').html(data.result.appRootTxHash);
             $('#root_app_addr').html(data.result.appRootAddr);
         }
    });
}
    
Applications.prototype.create_app = function (app_addr, app_name,locked, tx_fee) {
    var arAddr = [];
    var addr = $('#app_addr').val();
    var self = this;
    
    if ((addr == null) || (addr.length < 34)) {
        $('#app_error').html('no address selected');
        return;
    }
    
    if (app_name.length < 3) {
        $('#app_error').html('application name 3 char min.');
        return;
    }
    if (MyAccount.selected_balance < (tx_fee + root_app_fees)) {
        $('#app_error').html('not enough balance selected');
        return;
    }
    
    $('#app_error').empty();
    for (var n = 0; n < MyAccount.addrs.length; n++) {

        if (MyAccount.SelectedAddrs.indexOf(MyAccount.addrs[n].address) >= 0)
            arAddr.push(MyAccount.addrs[n].address);
    }


    rpc_call('makeapptx', [app_addr, app_name,locked, arAddr, tx_fee], function (data) {
        var html;
    
        my_tx = data.result.transaction;
        html  = get_tx_html(my_tx);
    
        $('#app_tx').html(html);
    });
}
    
Applications.prototype.create_app_type = function (type_name, type_id,  tx_fee) {
    var self = this;
    var arAddr = [];
    
    $('#app_error').css('display', 'none');
    
    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }
    
    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }
    
    rpc_call('makeapptypetx', [this.my_app.appName, type_name, type_id, this.newtype_keys, arAddr, tx_fee], function (data) {
        my_tx = data.result.transaction;
        $('#type_tx').html(get_tx_html(my_tx));
    });
}
    
    
Applications.prototype.create_app_file = function (fileHash, fileAddr, tx_fee) {
    var self = this;
    var arAddr = [];
    var DecHexkey = $('#selected_' + fileAddr).attr('privkey');
    
    if (DecHexkey == null) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('select private key for addr ' + fileAddr);
        return;
    }
    var mykey = ec.keyPair({ priv: DecHexkey, privEnc: 'hex' });
    var pubKey = $('#selected_' + fileAddr).attr('pubkey'); //mykey.getPublic().encodeCompressed('hex');
    var dpkey = mykey.getPublic().encodeCompressed('hex');
    var signature = mykey.sign(fileHash, 'hex');
    var derSign = signature.toLowS();
    
    
    $('#app_error').css('display', 'none');
    
    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }
    
    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }
    
    rpc_call('makeappfiletx', [self.my_app.appName, fileHash, pubKey, derSign, arAddr, tx_fee], function (data) {
        my_tx = data.result.transaction;
        $('#type_tx').html(get_tx_html(my_tx));
    });
    
}

Applications.prototype.create_app_layout = function (fileHash, tx_fee) {
    var self = this;
    var arAddr = [];
    
    $('#app_error').css('display', 'none');

    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }

    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }

    rpc_call('makeapplayouttx', [self.my_app.appName, fileHash, arAddr, tx_fee], function (data) {
        my_tx = data.result.transaction;
        $('#type_tx').html(get_tx_html(my_tx));
    });

}
    
Applications.prototype.create_app_module = function (fileHash, tx_fee) {
    var self = this;
    var arAddr = [];
    
    $('#app_error').css('display', 'none');

    if (MyAccount.addrs == null)
    {
        $('#app_error').css('display', 'block');
        $('#app_error').html('select an account');
        return;
    }

    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }

    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }

    rpc_call('makeappmoduletx', [self.my_app.appName, fileHash, arAddr, tx_fee], function (data) {
        my_tx = data.result.transaction;
        $('#type_tx').html(get_tx_html(my_tx));
    });

}
    
Applications.prototype.load_obj = function (objId, div_id) {
    rpc_call('loadobj', [this.my_app.appName, objId], function (data) {
        $('#' + div_id).html(JSON.stringify(data.result.obj));
    });
}


Applications.prototype.load_obj_p = function (objId,flags) {
    return rpc_call_promise('loadobj', [this.my_app.appName, objId, flags], true);

}

function updt_new_obj(data) {
    if (data.error) {
        my_tx = null;
        $('#type_tx').empty();

        $('#app_error').css('display', 'block');
        $('#app_error').html('error creating object tx.');
    }
    else {
        my_tx = data.result.transaction;
        $('#type_tx').html(get_tx_html(my_tx));
    }
}

Applications.prototype.create_app_obj = function (type_id, objAddr, newObj, tx_fee, done) {
    var self = this;
    var arAddr = [];
    var pubKey = $('#selected_' + objAddr).attr('pubkey');

    $('#app_error').css('display', 'none');

    if (MyAccount.addrs == null)
    {
        $('#app_error').css('display', 'block');
        $('#app_error').html('select the key for the obj addr ' + objAddr + '.');
        return;
    }

    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }

    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }

    if (pubKey == null) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('select the key for the obj addr ' + objAddr+'.');
        return;
    }

    if (done == null)
        done = updt_new_obj;

    rpc_call_promise('makeappobjtx', [self.my_app.appName, type_id, pubKey, newObj, arAddr, tx_fee],true).done(done);

}
    
function update_obj_child(data) {
    my_tx = data.result.transaction;
    $('#type_tx').html(get_tx_html(my_tx));
}

Applications.prototype.add_app_obj_child = function (objHash, keyName, childHash, tx_fee, done) {
     var self = this;
     var arAddr = [];

     $('#app_error').css('display', 'none');

    for (var n = 0; n < MyAccount.addrs.length; n++) {
        arAddr[n] = MyAccount.addrs[n].address;
    }

    if (MyAccount.selected_balance < tx_fee) {
        $('#app_error').css('display', 'block');
        $('#app_error').html('not enough balance selected');
        return;
    }

    if(done == null)
        done = update_obj_child;

    rpc_call_promise('addchildobj', [this.my_app.appName, objHash, keyName, childHash, arAddr, tx_fee]).done(done);
    

}
    
Applications.prototype.get_app_type_objs = function (type_id) {
     var self = this;

     rpc_call('get_type_obj_list', [this.my_app.appName, type_id], function (data) {

        if (!data.error) {
            self.update_app_type_obj(data.result.typeId, data.result.objs);
        }
        else {
            var tstr = type_id.toString(16);
            $('#objs_' + tstr).html('no objects');
        }


    });
}

Applications.prototype.get_app_type_objs_p = function (type_id) {
    
    return rpc_call_promise('get_type_obj_list', [this.my_app.appName, type_id]);
}

Applications.prototype.find_objs = function (type_id, addrs) {
    var arAddr=[];

    for (var n = 0; n < addrs.length; n++)
    {
        arAddr.push(addrs[n].address)
    }

    return rpc_call_promise('find_objs', [this.my_app.appName, type_id, arAddr], true);
}
Applications.prototype.listobjtxfr = function (type_id, objHash) {

    return rpc_call_promise('listobjtxfr', [this.my_app.appName, type_id, objHash], true);
}


    
Applications.prototype.get_app_file = function (file_hash) {
    var self = this;
    rpc_call('getappfile', [file_hash], function (data) {

        if (!data.error) {
            self.update_app_file(data.result.file, data.result.filePath);
        }
        else {
            $('#files').html('no files');
        }
    });
}
    
Applications.prototype.get_app_files = function () {
    var self = this;
    rpc_call('getappfiles', [this.my_app.appName], function (data) {

        if ((!data.error) && (data.result.total > 0)) {
            self.update_app_files(data.result.files, data.result.total);
        }
        else {
            $('#files').html('no files');
        }
    });
}
    
Applications.prototype.get_obj_type_name = function (type_id) {
    var n;

    if (type_id == 1)
        type_id = 0x0B000100;

    for (n = 0; n < this.nodeTypes.length; n++) {
        if (this.nodeTypes[n].id == type_id)
            return this.nodeTypes[n].name;
    }


    if (this.my_app == null) return null;
    if (this.my_app.app_types == null) return null;

    for (n = 0; n < this.my_app.app_types.length; n++) {
        if (this.my_app.app_types[n].id == type_id)
            return this.my_app.app_types[n].name;
    }
    return null;
}
    
Applications.prototype.get_obj_types = function (select_id) {
    var self = this;

   rpc_call('gettypes', [], function (data) {
       self.nodeTypes = data.result.types;
       self.update_types_select(select_id);
   });
}



Applications.prototype.setApp = function (app, nodeTypes, appTypes) {
    var app_div;

    this.my_app = app;
    
    if (this.my_app == null) {
         $('#app_infos').html('');
         $('#app_infos').css('display', 'none');
         $('#app_error').css('display', 'block');
         $('#app_name').html(app_name);
         return;
    }
    
    this.nodeTypes = nodeTypes;
    this.my_app.app_types = appTypes;
    
     $('#app_infos').css('display', 'block');
     $('#app_error').css('display', 'none');

     app_div = document.getElementById('app_hdr')

     if (app_div != null) {
         app_div.appendChild(this.get_app_header(this.my_app));
     }
    
     app_div = document.getElementById('app');
     if (app_div != null) {
         app_div.appendChild(this.get_app_section());

         this.update_types_select('new_type_key');
         this.update_app_types();

         if (this.my_app.app_types != null) {
             for (var n = 0; n < this.my_app.app_types.length; n++) {
                 this.get_app_type_objs(this.my_app.app_types[n].id);
             }
         }
     }
    
     this.get_app_files();
    
     if (typeof myfile != 'undefined')
         this.update_app_upl(myfile);
    
     if (typeof mylayout != 'undefined')
         this.update_app_upl_layout(mylayout);
    
     if (typeof mymod != 'undefined')
         this.update_app_upl_module(mymod);
}

Applications.prototype.get_apps_html = function () {
     var n = 0;
     var div = document.createElement('div');

     div.className = "container";
     div.id = "apps";
     if (this.apps != null) {
         for (n = 0; n < this.apps.length; n++) {
             div.appendChild(this.get_app_header(this.apps[n]));
         }
     }
     
     return div;
 }

Applications.prototype.get_apps_item_root = function (type) {
     var n = 0;

     for (n = 0; n < this.apps.txsout.length; n++) {
         if (this.apps.txsout[n].app_item == type)
             return n;
     }
     return 0;
 }

Applications.prototype.setApps = function (appList)
 {
    this.apps      = appList;
    this.my_app    = null;

    if (this.apps == null) return;

    document.getElementById('app_list').appendChild(this.get_apps_html());
 }

var MyApps = null;