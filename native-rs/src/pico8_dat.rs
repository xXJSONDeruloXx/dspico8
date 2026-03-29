#[derive(Clone, Debug, PartialEq, Eq)]
pub struct DatHit {
    pub offset: usize,
    pub tag: String,
    pub tokens: Vec<String>,
}

const TAGS: [&[u8; 5]; 1] = [&b"CPODD"];
const TAGS4: [&[u8; 4]; 3] = [&b"CFIL", &b"cFIL", &b"CBMP"];

pub fn scan_interesting_chunks(bytes: &[u8]) -> Vec<DatHit> {
    let mut hits = Vec::new();

    for tag in TAGS {
        for offset in find_all(bytes, tag) {
            let window_end = (offset + 128).min(bytes.len());
            let tokens = extract_tokens(&bytes[offset..window_end]);
            hits.push(DatHit {
                offset,
                tag: String::from_utf8_lossy(tag).into_owned(),
                tokens,
            });
        }
    }

    for tag in TAGS4 {
        for offset in find_all(bytes, tag) {
            let window_end = (offset + 160).min(bytes.len());
            let tokens = extract_tokens(&bytes[offset..window_end]);
            hits.push(DatHit {
                offset,
                tag: String::from_utf8_lossy(tag).into_owned(),
                tokens,
            });
        }
    }

    hits.sort_by_key(|hit| hit.offset);
    hits
}

fn find_all(bytes: &[u8], needle: &[u8]) -> Vec<usize> {
    let mut hits = Vec::new();
    let mut pos = 0usize;
    while pos + needle.len() <= bytes.len() {
        if &bytes[pos..pos + needle.len()] == needle {
            hits.push(pos);
        }
        pos += 1;
    }
    hits
}

fn extract_tokens(window: &[u8]) -> Vec<String> {
    let mut current = Vec::new();
    let mut tokens = Vec::new();

    let flush = |current: &mut Vec<u8>, tokens: &mut Vec<String>| {
        if current.len() >= 4 {
            let token = String::from_utf8_lossy(current).into_owned();
            if !tokens.contains(&token) {
                tokens.push(token);
            }
        }
        current.clear();
    };

    for &byte in window {
        let ok = matches!(byte,
            b'A'..=b'Z' |
            b'a'..=b'z' |
            b'0'..=b'9' |
            b'_' | b'.' | b'/' | b'\\' | b'-'
        );
        if ok {
            current.push(byte);
        } else {
            flush(&mut current, &mut tokens);
        }
    }
    flush(&mut current, &mut tokens);

    tokens.retain(|token| {
        token.contains('/')
            || token.contains(".p8")
            || token.contains(".pod")
            || token.contains(".js")
            || token.contains(".wasm")
            || token.contains("pico8")
            || token == "CFIL"
            || token == "cFIL"
            || token == "CPODD"
            || token == "CBMP"
    });

    tokens
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn finds_tag_hits_in_sample_bytes() {
        let bytes = b"xxxxCPODD....CFIL....pod/pico8_boot.p8....cFIL....src/pico8.js";
        let hits = scan_interesting_chunks(bytes);
        assert!(hits.iter().any(|hit| hit.tag == "CPODD"));
        assert!(hits.iter().any(|hit| hit.tag == "CFIL"));
        assert!(hits.iter().any(|hit| hit
            .tokens
            .iter()
            .any(|token| token.contains("pico8_boot.p8"))));
    }
}
