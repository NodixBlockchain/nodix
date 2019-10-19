var AudioContext = window.AudioContext || window.webkitAudioContext;

function webAudio() {
}

webAudio.prototype.playbuffer = function (audioCtx, MySample) {
    var AudioBuffer = audioCtx.createBuffer(2, MySample.samples.length, MySample.samplerate);
    for (var canal = 0; canal < 2; canal++) {
        var tampon = AudioBuffer.getChannelData(canal);
        for (var i = 0; i < MySample.samples.length; i++) {
            tampon[i] = MySample.samples[i];
        }
    }
    return AudioBuffer;
}
webAudio.prototype.playSample = function (audioCtx,MySample) {

    var source = audioCtx.createBufferSource();
    source.buffer = this.playbuffer(audioCtx,MySample);
    source.connect(audioCtx.destination);
    source.start();
}

var MyAudio = null;
