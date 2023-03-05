#![allow(non_snake_case, unused_macros)]

use once_cell::unsync::Lazy;
use proconio::input;
use rand::prelude::*;
use svg::node::{
    element::{Group, Rectangle, Title},
    Text,
};

pub trait SetMinMax {
    fn setmin(&mut self, v: Self) -> bool;
    fn setmax(&mut self, v: Self) -> bool;
}
impl<T> SetMinMax for T
where
    T: PartialOrd,
{
    fn setmin(&mut self, v: T) -> bool {
        *self > v && {
            *self = v;
            true
        }
    }
    fn setmax(&mut self, v: T) -> bool {
        *self < v && {
            *self = v;
            true
        }
    }
}

#[macro_export]
macro_rules! mat {
	($($e:expr),*) => { Vec::from(vec![$($e),*]) };
	($($e:expr,)*) => { Vec::from(vec![$($e),*]) };
	($e:expr; $d:expr) => { Vec::from(vec![$e; $d]) };
	($e:expr; $d:expr $(; $ds:expr)+) => { Vec::from(vec![mat![$e $(; $ds)*]; $d]) };
}

#[derive(Clone, Copy, Debug)]
pub struct Cargo {
    pub w: i32,
    pub h: i32,
    pub d: i32,
    pub a: i32,
    pub f: char,
    pub g: char,
}

pub const CARGO: Lazy<Vec<(i32, i32, i32)>> = Lazy::new(|| {
    let csv = include_str!("./input.csv");
    let mut cs = vec![];
    for line in csv.lines().skip(1) {
        let line = line.trim();
        if line.len() > 0 {
            let ss = line.split(',').collect::<Vec<_>>();
            cs.push((ss[0].parse().unwrap(), ss[1].parse().unwrap(), ss[2].parse().unwrap()));
        }
    }
    cs
});

#[derive(Clone, Copy, Debug)]
pub struct Placement {
    pub p: usize,
    pub r: i32,
    pub x: i32,
    pub y: i32,
    pub z: i32,
    pub occupy: [(i32, i32); 3],
}

impl Placement {
    fn new(input: &Input, p: usize, r: i32, x: i32, y: i32, z: i32) -> Self {
        let Cargo { w, h, d, .. } = input.cs[p];
        let (dx, dy, dz) = match r {
            0 => (w, h, d),
            1 => (h, w, d),
            2 => (d, h, w),
            3 => (h, d, w),
            4 => (d, w, h),
            5 => (w, d, h),
            _ => unreachable!(),
        };
        let occupy = [(x, x + dx), (y, y + dy), (z, z + dz)];
        Self { p, r, x, y, z, occupy }
    }
}

pub type Output = Vec<Placement>;

#[derive(Clone, Debug)]
pub struct Input {
    pub M: usize,
    pub W: i32,
    pub H: i32,
    pub B: i32,
    pub D: i32,
    pub cs: Vec<Cargo>,
}

impl std::fmt::Display for Input {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(fmt, "{} {} {} {} {}", self.M, self.W, self.H, self.B, self.D)?;
        for &Cargo { h, w, d, a, f, g } in &self.cs {
            writeln!(fmt, "{} {} {} {} {} {}", h, w, d, a, f, g)?;
        }
        Ok(())
    }
}

pub fn parse_input(f: &str) -> Input {
    let f = proconio::source::once::OnceSource::from(f);
    input! {
        from f,
        M: usize, W: i32, H: i32, B: i32, D: i32,
        cs: [(i32, i32, i32, i32, char, char); M],
    }
    let cs = cs.into_iter().map(|(h, w, d, a, f, g)| Cargo { h, w, d, a, f, g }).collect();
    Input { M, W, H, B, D, cs }
}

fn read<T: Copy + PartialOrd + std::fmt::Display + std::str::FromStr>(v: &str, lb: T, ub: T) -> Result<T, String> {
    if let Ok(v) = v.parse::<T>() {
        if v < lb || ub < v {
            Err(format!("Out of range: {}", v))
        } else {
            Ok(v)
        }
    } else {
        Err(format!("Parse error: {}", v))
    }
}

pub fn parse_output(input: &Input, f: &str) -> Result<Output, String> {
    let mut out = vec![];
    for line in f.lines() {
        let line = line.trim();
        if line.len() > 0 {
            let tokens = line.split_whitespace().collect::<Vec<_>>();
            if tokens.len() != 5 {
                return Err(format!("Illegal output format: {}", line));
            }
            out.push(Placement::new(
                input,
                read(tokens[0], 0, input.M - 1)?,
                read(tokens[1], 0, 5)?,
                read(tokens[2], 0, input.W)?,
                read(tokens[3], 0, input.H)?,
                read(tokens[4], 0, 1000000000)?,
            ));
        }
    }
    Ok(out)
}

fn common(a: &[(i32, i32)], b: &[(i32, i32)]) -> i64 {
    let mut v = 1;
    for i in 0..a.len() {
        let left = a[i].0.max(b[i].0);
        let right = a[i].1.min(b[i].1);
        if left < right {
            v *= (right - left) as i64;
        } else {
            return 0;
        }
    }
    v
}

pub fn compute_score(input: &Input, out: &[Placement], sub: bool) -> (i64, String) {
    let mut count = vec![0; input.M];
    for i in 0..out.len() {
        let Placement { p, r, x, y, z, occupy } = out[i];
        count[p] += 1;
        if count[p] > input.cs[p].a {
            return (0, format!("荷物の総数が超過しました。 (i = {}, p = {})", i, p));
        }
        if input.cs[p].f == 'N' && r != 0 && r != 1 {
            return (0, format!("荷物 {} の底面を変えることは出来ません。", i));
        }
        let (x2, y2, _z2) = (occupy[0].1, occupy[1].1, occupy[2].1);
        if x2 > input.W || y2 > input.H {
            return (0, format!("荷物 {} がコンテナに収まっていません。", i));
        }
        for b in &[
            [(0, input.B), (0, input.B)],
            [(0, input.B), (input.H - input.B, input.H)],
            [(input.W - input.B, input.W), (0, input.B)],
            [(input.W - input.B, input.W), (input.H - input.B, input.H)],
        ] {
            if common(&[(x, x2), (y, y2)], b) > 0 {
                return (0, format!("荷物 {} がコンテナに収まっていません。", i));
            }
        }
        let mut area = 0;
        for j in 0..i {
            if common(&occupy, &out[j].occupy) > 0 {
                return (0, format!("荷物 {} と荷物 {} が重なっています。", i, j));
            } else if common(&[(x, x2), (y, y2), (z, 1000000000)], &out[j].occupy) > 0 {
                return (0, format!("荷物 {} の上部に荷物 {} が既に存在しています。", i, j));
            } else if z == out[j].occupy[2].1 {
                let a = common(&[(x, x2), (y, y2)], &out[j].occupy[0..2]);
                if a > 0 {
                    if input.cs[out[j].p].g == 'N' {
                        return (0, format!("荷物 {} を荷物 {} の上に置くことは出来ません。", i, j));
                    }
                    area += a;
                }
            }
        }
        if z > 0 && area < ((x2 - x) * (y2 - y)) as i64 * 6 / 10 {
            return (0, format!("荷物 {} の底面が十分な面積接地していません。", i));
        }
    }
    if !sub {
        for p in 0..input.M {
            if count[p] != input.cs[p].a {
                return (0, format!("荷物の総数が足りません (p = {})", p));
            }
        }
    }
    let mut score = 1000;
    let max_height = out.iter().map(|c| c.occupy[2].1).max().unwrap_or(0);
    score += max_height as i64;
    for i in 0..out.len() {
        for j in i + 1..out.len() {
            if out[i].p > out[j].p {
                score += 1000;
            }
        }
    }
    if max_height > input.D {
        score += 1000000;
        for c in out {
            if c.occupy[2].1 > input.D {
                score += 1000
                    * (c.occupy[0].1 - c.occupy[0].0) as i64
                    * (c.occupy[1].1 - c.occupy[1].0) as i64
                    * (c.occupy[2].1 - c.occupy[2].0) as i64;
            }
        }
    }
    (score, String::new())
}

pub fn gen(seed: u64) -> Input {
    let mut rng = rand_chacha::ChaCha20Rng::seed_from_u64(seed);
    let W = 1120;
    let H = 680;
    let B = 30;
    let D = rng.gen_range(1, 3) * 600;
    let V = (W * H * D - 4 * B * B * D) as i64;
    let Vmin = rng.gen_range((3 * V + 9) / 10, 8 * V / 10);
    let Vmax = Vmin + V / 10;
    let mut cs = vec![];
    loop {
        let mut total_vol = 0;
        while total_vol < Vmin {
            let &(w, h, d) = CARGO.choose(&mut rng).unwrap();
            let vol = w * h * d;
            let a_max = if vol >= 50000000 {
                1
            } else if vol >= 10000000 {
                3
            } else if vol >= 2500000 {
                10
            } else {
                30
            };
            let a = *(1..=a_max)
                .collect::<Vec<_>>()
                .choose_weighted(&mut rng, |a| 1.0 / (a * a) as f64)
                .unwrap();
            cs.push(Cargo {
                w,
                h,
                d,
                a,
                f: 'Y',
                g: 'Y',
            });
            total_vol += vol as i64 * a as i64;
        }
        if total_vol <= Vmax {
            break;
        }
        cs.clear();
    }
    let F = rng.gen_range(0.0, 0.3);
    for c in &mut cs {
        if rng.gen_bool(F) {
            c.f = 'N';
        }
    }
    let mut total_area = 0;
    let area_ub = (W * H - 4 * B * B) * 6 / 10;
    while rng.gen_bool(0.5) {
        if let Some(i) = (0..cs.len()).filter(|&i| cs[i].g == 'Y').choose(&mut rng) {
            total_area += cs[i].w * cs[i].h;
            if total_area > area_ub {
                break;
            }
            cs[i].g = 'N';
        } else {
            break;
        }
    }
    Input {
        M: cs.len(),
        W,
        H,
        B,
        D,
        cs,
    }
}

/// 0 <= val <= 1
fn color(mut val: f64) -> String {
    val.setmin(1.0);
    val.setmax(0.0);
    let (r, g, b) = if val < 0.5 {
        let x = val * 2.0;
        (
            30. * (1.0 - x) + 144. * x,
            144. * (1.0 - x) + 255. * x,
            255. * (1.0 - x) + 30. * x,
        )
    } else {
        let x = val * 2.0 - 1.0;
        (
            144. * (1.0 - x) + 255. * x,
            255. * (1.0 - x) + 30. * x,
            30. * (1.0 - x) + 70. * x,
        )
    };
    format!("#{:02x}{:02x}{:02x}", r.round() as i32, g.round() as i32, b.round() as i32)
}

fn rect(x: i32, y: i32, w: i32, h: i32, fill: &str) -> Rectangle {
    Rectangle::new()
        .set("x", x)
        .set("y", y)
        .set("width", w)
        .set("height", h)
        .set("fill", fill)
}

pub fn vis_default(input: &Input, out: &Output) -> String {
    vis(input, out, 0)
}

pub fn vis(input: &Input, out: &[Placement], color_type: i32) -> String {
    let mut doc = svg::Document::new()
        .set("id", "vis")
        .set("viewBox", (-5, -5, input.W + 10, input.H + 10))
        .set("width", input.W + 10)
        .set("height", input.H + 10);
    doc = doc.add(rect(-5, -5, input.W + 10, input.H + 10, "white"));
    doc = doc.add(
        rect(0, 0, input.W, input.H, "white")
            .set("stroke", "black")
            .set("stroke-width", 2),
    );
    for &(x, y) in &[
        (0, 0),
        (0, input.H - input.B),
        (input.W - input.B, 0),
        (input.W - input.B, input.H - input.B),
    ] {
        let group = Group::new().add(Title::new().add(Text::new(format!(
            "Block\n[{}, {}] × [{}, {}] × [{}, {}]",
            x,
            x + input.B,
            y,
            y + input.B,
            0,
            input.D
        ))));
        doc = doc.add(
            group.add(
                rect(x, input.H - y - input.B, input.B, input.B, "#606060")
                    .set("stroke", "black")
                    .set("stroke-width", 2),
            ),
        );
    }
    for i in 0..out.len() {
        let [(x, x2), (y, y2), (z, z2)] = out[i].occupy;
        let group = Group::new().add(Title::new().add(Text::new(format!(
            "i={}, p={}, r={}, x={}, y={}, z={}\n[{}, {}] × [{}, {}] × [{}, {}]",
            i, out[i].p, out[i].r, x, y, z, x, x2, y, y2, z, z2
        ))));
        doc = doc.add(
            group.add(
                rect(
                    x,
                    input.H - y2,
                    x2 - x,
                    y2 - y,
                    &color(if color_type == 0 {
                        z2 as f64 / (input.D as f64 * 1.5)
                    } else {
                        (out[i].p as f64 + 0.5) / input.M as f64
                    }),
                )
                .set("stroke", if z2 > input.D { "red" } else { "black" })
                .set("stroke-width", if z2 > input.D { 4 } else { 2 }),
            ),
        )
    }
    doc.to_string()
}
