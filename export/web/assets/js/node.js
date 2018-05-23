
class Node
{
    make_node_html(name, node) {
        var html = '<div class="container">';

        html += make_var_html("user agent", node.user_agent);

        if (typeof node.p2p_addr != 'undefined') {
            html += make_var_html("address", node.p2p_addr.addr);
            html += make_var_html("port", node.p2p_addr.port);
        }
        html += make_var_html("version", node.version);
        html += make_var_html("height", node.block_height);
        html += make_var_html("ping", node.ping_delay + ' ms');

        if (typeof (node.last_block) != 'undefined')
            html += '<div class="row"><div class="col-md-2"><label>last block</label></div><div class="col-md-2"><a class="hash-lnk" >' + node.last_block + '</a></div></div>';

        html += '</div>';

        $('#' + name).html(html);
        
    }

    
    make_modules_html(name, modules) {
        var self = this;

        $('#' + name).empty();
        for (var i = 0; i < modules.length; i++) {
            var html = '';

            if (typeof modules[i].module != 'undefined')
                var mDef = modules[i].module;
            else
                var mDef = modules[i];

            html = '<section>'
            html += '<h3 style="cursor:pointer" onclick="$(\'#mod_infos_' + mDef.name + '\').toggle();" >';
            html += '<img src="/assets/img/mod.gif" alt="module" />';
            html += '<span class="medium">' + mDef.name + '</span>';
            if (typeof modules[i].base != 'undefined') html += '&nbsp;' + modules[i].base;
            html += '</h3>';

            html += '<div style="display:none;" id="mod_infos_' + mDef.name + '" >';
            html += '<div><span>file :</span><span>' + mDef.file + '</span></div>';
            html += '<div><span>size :</span><span>' + mDef.size + '</span>&nbsp;bytes</div>';

            html += '<h4>methods</h4>';

            html += '<ul id="mod_procs_' + mDef.name + '" style="padding-left:12px; list-style: square inside url(/assets/img/proc.gif);"  ;>'
            for (var n = 0; n < mDef.exports.length; n++) {
                html += '<li  method="' + mDef.exports[n] + '">'
                if (modules[i].type == 'cgi')
                    html += '<a onclick="click_cgi_method(MyNode.mods[' + i + '],\'' + mDef.exports[n] + '\'); return false;" href="' + modules[i].base + mDef.exports[n] + '" >' + mDef.exports[n] + '</a>';
                else if (modules[i].type == 'rpc')
                    html += '<a onclick="click_rpc_method(MyNode.mods[' + i + '],\'' + mDef.exports[n] + '\'); " href="#api_div" >' + mDef.exports[n] + '</a>';
                else
                    html += '<a >' + mDef.exports[n] + '</a>';

                html += '<span  class="args"></span>';
                html += '<p  class="desc"></p>';


                html += '</li>';
            }
            html += '</ul>'

            html += '</div>'
            html += '</section>';
            $('#' + name).append(html);
        }
    }


    get_node_lag(node) {
        var n;
        var height = 0;
        for (n = 0; n < node.peer_nodes.length; n++) {
            height = Math.max(height, node.peer_nodes[n].block_height + 1);
        }
        var now = Math.floor(new Date().getTime() / 1000);
        var diff = now - node.last_block_time;
        var msec = diff;
        var dd = Math.floor(msec / (60 * 60 * 24));
        msec -= dd * 60 * 60 * 24;
        var hh = Math.floor(msec / (60 * 60));

        $('#node_time_lag').html(dd + ' days, ' + hh + 'hours');

        if (node.block_height <= height)
            $('#node_block_lag').html(height - node.block_height + '&nbsp; blocks behind');
        else
            $('#node_block_lag').html(node.block_height - height + '&nbsp; blocks ahead');

    }

    update_mempool_txs(txs, tbl_name) {
        var n;

        txs.sort(function (a, b) { return (b.time - a.time); });

        if (document.getElementById(tbl_name).tBodies) {
            var old_tbody = document.getElementById(tbl_name).tBodies[0];
            var new_tbody = document.createElement('tbody');

            for (n = 0; n < txs.length; n++) {
                var row = new_tbody.insertRow(n * 2);
                var cell;
                row.className = "txhdr";

                cell = row.insertCell(0);
                cell.className = "tx_hash";
                cell.innerHTML = '<span class="tx_expand" onclick="show_tx(\'' + txs[n].txid + '\'); if(this.innerHTML==\'+\'){ this.innerHTML=\'-\'; }else{ this.innerHTML=\'+\'; } ">+</span><a class="tx_lnk" onclick="SelectTx(\'' + txs[n].txid + '\'); return false;" href="' + site_base_url + '/tx/' + txs[n].txid + '">' + '#' + n + '</a>&nbsp;' + timeConverter(txs[n].time);

                row = new_tbody.insertRow(n * 2 + 1);

                row.id = "tx_infos_" + txs[n].txid;
                row.className = "tx_infos";


                if (txs[n].is_app_root == 1) {
                    cell = row.insertCell(0);
                    cell.className = "txins";
                    cell.innerHTML = 'approot <br/>';
                    cell = row.insertCell(1);
                    cell.className = "txouts";
                    cell.innerHTML = '#0 ' + txs[n].vout[0].value + ' ' + txs[n].dstaddr + '<br/>';
                }
                else {
                    cell = row.insertCell(0);
                    cell.className = "txins";

                    var nins, nouts;
                    var html = '';

                    nins = txs[n].vin.length;
                    for (nn = 0; nn < nins; nn++) {
                        if (txs[n].vin[nn].srcapp) {
                            new_html += 'App&nbsp; : ' + vin[nn].srcapp;

                            if (txs[n].appChildOf) new_html += '&nbsp; child';
                            if (txs[n].app_item == 1) new_html += '&nbsp; type';
                            if (txs[n].app_item == 2) new_html += '&nbsp; obj';
                            if (txs[n].app_item == 3) new_html += '&nbsp; file';

                        }
                        else {
                            var hh = ' <a href= "' + site_base_url + '/address/' + txs[n].vin[nn].dstaddr + '" class="tx_address">' + txs[n].vin[nn].dstaddr + '</a>';
                            html += '#' + nn + '&nbsp' + hh + '&nbsp' + txs[n].vin[nn].value / unit + '  <br/>';
                        }
                    }
                    cell.innerHTML = html;

                    cell = row.insertCell(1);
                    cell.className = "txouts";

                    html = '';
                    if (txs[n].vout) {
                        nouts = txs[n].vout.length;
                        for (nn = 0; nn < nouts; nn++) {
                            var hh = ' <a href="' + site_base_url + '/address/' + txs[n].vout[nn].dstaddr + '" class="tx_address">' + txs[n].vout[nn].dstaddr + '</span> ';
                            html += '#' + nn + '&nbsp' + hh + '&nbsp' + txs[n].vout[nn].value / unit + ' <br/>';
                        }
                    }
                    cell.innerHTML = html;
                }
            }
            old_tbody.parentNode.replaceChild(new_tbody, old_tbody);
        }
        else {
            var new_html = '';
            $('#' + tbl_name).empty();
            for (n = 0; n < txs.length; n++) {
                new_html += get_tx_html(txs[n], n);
            }
            $('#' + tbl_name).html(new_html);
        }
    }



    fill_module_def_html(def) {

        var self = this;

        this.modules_definitions[def.name] = def;

        $('#mod_infos_' + def.name).prepend('<h3>' + def.desc + '</h3>');
        $('#mod_procs_' + def.name + ' li').each(function (index) {
            var proc = self.find_proc_name(def, $(this).attr('method'));
            if (proc != null) {

                $(this).find('.desc').html(proc.desc);
                var args = '(';
                if (typeof proc.params != 'undefined') {
                    var first = 1;
                    for (var i = 0; i < proc.params.length; i++) {
                        if (!first)
                            args += ',&nbsp;';

                        args += proc.params[i].name;
                        args += '<span style="font-size:8px;">' + proc.params[i].desc + '</span>';
                        first = 0;
                    }
                }
                args += ')';
                $(this).find('.args').html(args);
            }
        })
    }

    
    make_scripts_html(name, script_list) {
        var scripts = script_list;

        $('#' + name).empty();
        
        for (var i = 0; i < scripts.length; i++) {
            var html = '';
            html = '<div>'
            html += '<section>'
            html += '<h1><strong>' + scripts[i].file + '</strong></h1>';
            for (var scriptk in scripts[i]) {
                var script_var = scripts[i][scriptk];
                html += '<div>';
                html += '<label  ><h3 onclick=" $(\'#var_' + scriptk + '\').slideToggle();">' + scriptk + '</h3></label>';
                if ((typeof script_var == 'object') || (typeof script_var == 'Array') || (script_var.length < 32)) {
                    if (typeof script_var == 'object') {
                        for (var ne = 0; ne < script_var.length; ne++) {
                            html += '<div>' + script_var[ne] + '</div>';
                        }
                    }
                    else
                        html += '&nbsp;:&nbsp;<span>' + script_var + '</span>';
                }

                else {
                    var str = '';

                    if (typeof script_var == 'string')
                        str = script_var.replace(/(?:\r\n|\r|\n)/g, '<br />');
                    else if (typeof script_var == 'integer')
                        str = script_var.toString();

                    html += '<div id="var_' + scriptk + '" class="script_proc">' + str + '</div>';
                }
                html += '</div>'
            }
            html += '</section>';
            html += '</div>'
            $('#' + name).append(html);
        }
    }


    make_handlers_html(name, handlers_list) {
        $('#' + name).empty();
        this.handlers = handlers_list;
        var html = '';
        html = '<div>'
        html += '<section>'
        for (var handlerk in this.handlers) {
            var script_var = this.handlers[handlerk];
            var str = script_var.replace(/(?:\r\n|\r|\n)/g, '<br />');
            html += '<div>';
            html += '<label  ><h3 onclick=" $(\'#msg_' + handlerk + '\').slideToggle();">' + handlerk + '(node,payload)</h3></label>';
            html += '<div id="msg_' + handlerk + '" class="script_proc">' + str + '</div>';
            html += '</div>'
        }
        html += '</section>';
        html += '</div>'
        $('#' + name).append(html);

    }

    find_proc_name(def, proc_name) {
        if (typeof def.methods != 'undefined') {
            for (var n = 0; n < def.methods.length; n++) {
                if (def.methods[n].name == proc_name) {
                    return def.methods[n];
                }
            }
        }
        return null;
    }

    make_mime_table(div_name, mimes) {
        var html = '<table class="table" >';
        html += '<thead><tr><th>extension</th><th>mime</th></tr></thead>';
        html += '<tbody>';
        for (var mimek in mimes) {
            html += '<tr><td>' + mimek + '</td><td>' + mimes[mimek] + '</td></tr>';
        }
        html += '</tbody>';
        html += '</table>';
        $('#' + div_name).html(html);
    }

    setInfos(node)
    {

        $('#node_name').html(node.user_agent);
        $('#node_version').html(node.version);
        $('#node_bheight').html(node.block_height);

        $('#node_port').html(node.p2p_addr.port);
        $('#node_addr').html(node.p2p_addr.addr);

        $('#lastnodeblock').html(node.last_block);
        /* $('#lastnodeblock').attr('href', '/nodix.site/block/' + node.last_block); */
        $('#lastnodeblock').attr('data-target', '#blockmodal');
        $('#lastnodeblock').attr('data-toggle', 'modal');

        $('#lastpowblock').html(node.lastPOWBlk);
        /* $$('#lastpowblock').attr('href', '/nodix.site/block/' + node.lastPOWBlk); */
        $('#lastpowblock').attr('data-target', '#blockmodal');
        $('#lastpowblock').attr('data-toggle', 'modal');

        $('#lastposblock').html(node.lastPOSBlk);
        /* $('#lastposblock').attr('href', '/nodix.site/block/' + node.lastPOSBlk); */
        $('#lastposblock').attr('data-target', '#blockmodal');
        $('#lastposblock').attr('data-toggle', 'modal');

        $('#currentpowdiff').html(node.current_pow_diff.toString(16));
        $('#currentposdiff').html(node.current_pos_diff.toString(16));

        $('#powreward').html(node.pow_reward);
        $('#posreward').html(node.pos_reward);
    }

    constructor() {
        this.mods = null;
        this.handlers = null;
        this.modules_definitions = {};
    }
}

var MyNode = null;